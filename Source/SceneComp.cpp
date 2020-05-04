/*
 Acoustic scene components
 
 Authors:
 Matteo Scerbo (matteo.scerbo@mail.polimi.it)
 Luca Bondi (luca.bondi@polimi.it)
 */

#include "SceneComp.h"

void TileComp::resized(){
    area = getLocalBounds();
    frontFacingTransf = AffineTransform::rotation(MathConstants<float>::pi, area.getWidth() / 2, area.getHeight() / 2);
}

void TileComp::paint(Graphics &g) {
    Path path;
    
    path.startNewSubPath(corners[0][0]);
    path.lineTo(corners[1][0]);
    path.lineTo(corners[1][1]);
    path.lineTo(corners[0][1]);
    path.closeSubPath();
    
    if ((frontFacing != nullptr) && (bool) *frontFacing) {
        //TODO: Move this to ComputeVertices in Grid Component
        path.applyTransform(frontFacingTransf);
    }
    
    g.setColour(tileColour);
    g.fillPath(path);
    
    g.setColour(Colours::black);
    PathStrokeType strokeType(0.5f);
    g.strokePath(path, strokeType);
}

void TileComp::setFrontFacingParam(const std::atomic<float> *p) {
    frontFacing = p;
}

void TileComp::setColour(const Colour &col) {
    tileColour = col;
}

void TileComp::setCorners(const juce::Point<float> &p1,
                          const juce::Point<float> &p2,
                          const juce::Point<float> &p3,
                          const juce::Point<float> &p4) {
    corners[0][0] = p1;
    corners[1][0] = p2;
    corners[0][1] = p3;
    corners[1][1] = p4;
}
//==============================================================================
//==============================================================================

GridComp::GridComp() {
    for (int i = 0; i < TILE_ROW_COUNT; i++)
        for (int j = 0; j < NUM_DOAS; j++)
            addAndMakeVisible(tiles[i][j]);
    
    energy.resize(NUM_DOAS);
    energyPreGain.resize(NUM_DOAS, -30);
    
    
    // Compute led tresholds
    const float ledStep = 3; //dB
    
    th.clear();
    for (auto ledIdx = TILE_ROW_COUNT - 1; ledIdx >= 0; --ledIdx) {
        auto ledThDb = ledIdx == (TILE_ROW_COUNT - 1) ? RED_LT : -((TILE_ROW_COUNT - 1 - ledIdx) * ledStep);
        th.push_back(ledThDb);
    }
    
    startTimerHz(gridUpdateFrequency);
    
}

void GridComp::setCallback(const Callback *c) {
    callback = c;
}

void GridComp::setParams(const std::atomic<float> *frontFacing) {
    for (int i = 0; i < TILE_ROW_COUNT; i++) {
        for (int j = 0; j < NUM_DOAS; j++) {
            tiles[i][j].setFrontFacingParam(frontFacing);
        }
    }
}

void GridComp::resized() {
    
    area = getLocalBounds();
    
    computeVertices();
    
    for (int i = 0; i < TILE_ROW_COUNT; i++) {
        for (int j = 0; j < NUM_DOAS; j++) {
            
            tiles[i][j].setBounds(area);
            
            tiles[i][j].setCorners(vertices[i][j], vertices[i + 1][j], vertices[i][j + 1], vertices[i + 1][j + 1]);
            
            if (i < TILE_ROW_COUNT / 4)
                tiles[i][j].setColour(Colours::red.darker(0.9));
            
            if (TILE_ROW_COUNT / 4 <= i && i < TILE_ROW_COUNT / 2)
                tiles[i][j].setColour(Colours::yellow.darker(0.9));
            
            if (i >= TILE_ROW_COUNT / 2)
                tiles[i][j].setColour(Colours::green.darker(0.9));
            
        }
    }
}

void GridComp::timerCallback() {
    
    std::vector<float> newEnergy(NUM_DOAS);
    callback->getDoaEnergy(newEnergy);
    
    for (auto dirIdx = 0; dirIdx < energyPreGain.size(); ++dirIdx) {
        energyPreGain[dirIdx] = ((1 - inertia) * (newEnergy[dirIdx])) + (inertia * energyPreGain[dirIdx]);
    }
    
    // Very basic automatic gain
    auto rangeEnergy = FloatVectorOperations::findMinAndMax(energyPreGain.data(), (int) energyPreGain.size());
    auto maxLevel = rangeEnergy.getEnd() + gain;
    
    if (maxLevel > 0) {
        gain = jmax(gain - maxLevel - 3, minGain);
    } else if (maxLevel < -18) {
        gain = jmin(gain - maxLevel, maxGain);
    } else if (maxLevel > -3) {
        gain = jmax(gain - 0.5f, minGain);
    } else if (maxLevel < -9) {
        gain = jmin(gain + 0.5f, maxGain);
    }
    
    for (auto dirIdx = 0; dirIdx < energyPreGain.size(); ++dirIdx) {
        energy[dirIdx] = energyPreGain[dirIdx] + gain;
    }
    
    for (int j = 0; j < NUM_DOAS; j++) {
        for (int i = 0; i < TILE_ROW_COUNT; i++) {
            tiles[i][j].setColour(SingleChannelLedBar::thToColour(th[i], energy[j] > th[i]));
        }
    }
    
    repaint();
    
}

void GridComp::computeVertices() {
    float angle_diff = MathConstants<float>::pi / NUM_DOAS;
    
    for (int i = 0; i <= TILE_ROW_COUNT; i++) {
        
        const float radius = jmin(area.getHeight(),area.getWidth()/2) * (1 - (exp((float) i / TILE_ROW_COUNT) - 1) / (exp(1) - 1));
        
        for (int j = 0; j <= NUM_DOAS; j++) {
            const float angle = j * angle_diff;
            
            vertices[i][j].setX(area.getWidth() / 2 - radius * cos(angle));
            vertices[i][j].setY(area.getHeight() - radius * sin(angle));
            
        }
    }
}

//==============================================================================
//==============================================================================

void BeamComp::setParams(const std::atomic<float> *frontFacing,
                         const std::atomic<float> *mute,
                         const std::atomic<float> *width,
                         const std::atomic<float> *steer) {
    muteParam = mute;
    widthParam = width;
    steerParam = steer;
    frontFacingParam = frontFacing;
}

void BeamComp::resized(){
    area = getLocalBounds();
}

void BeamComp::paint(Graphics &g) {
    
    const float width = (0.1 + 2.9 * (*widthParam)) * area.getWidth() / 10;
    const float position = *steerParam;
    
    Path path;
    path.startNewSubPath(0, 0);
    path.cubicTo(width, -area.getWidth() / 3, width, -area.getWidth() / 2, 0, -area.getWidth() / 2);
    path.cubicTo(-width, -area.getWidth() / 2, -width, -area.getWidth() / 3, 0, 0);
    path.closeSubPath();
    
    path.applyTransform(AffineTransform::rotation((MathConstants<float>::pi / 2) * position));
    path.applyTransform(AffineTransform::translation(area.getWidth() / 2, area.getHeight()));
    
    if ((bool) *frontFacingParam) {
        path.applyTransform(AffineTransform::verticalFlip(area.getHeight()));
    }
    
    if (~(bool) *muteParam) {
        g.setColour(baseColour.brighter());
        g.setOpacity(0.4);
        g.fillPath(path);
    }
    
    g.setColour(baseColour);
    g.setOpacity(0.8);
    PathStrokeType strokeType(2);
    g.strokePath(path, strokeType);
}

//==============================================================================
//==============================================================================

SceneComp::SceneComp() {
    addAndMakeVisible(grid);
    for (int i = 0; i < NUM_BEAMS; i++)
        addAndMakeVisible(beams[i]);
}

void SceneComp::setCallback(const Callback *c) {
    grid.setCallback(c);
    grid.setParams(c->getFrontFacingParam());
    
    for (auto idx = 0; idx < NUM_BEAMS; idx++) {
        beams[idx].setParams(c->getFrontFacingParam(), c->getBeamMute(idx), c->getBeamWidth(idx), c->getBeamSteerX(idx));
    }
}

void SceneComp::paint(Graphics &g) {
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

void SceneComp::resized() {
    auto sceneArea = getLocalBounds();
    grid.setBounds(sceneArea);
    for (auto &b: beams)
        b.setBounds(sceneArea);
}

void SceneComp::setBeamColors(const std::vector<Colour> &colours) {
    jassert(colours.size() == NUM_BEAMS);
    for (auto beamIdx = 0; beamIdx < NUM_BEAMS; ++beamIdx) {
        beams[beamIdx].setBaseColor(colours[beamIdx]);
    }
}

//==============================================================================
//==============================================================================
