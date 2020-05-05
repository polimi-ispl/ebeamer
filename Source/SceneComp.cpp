/*
 Acoustic scene components
 
 Authors:
 Matteo Scerbo (matteo.scerbo@mail.polimi.it)
 Luca Bondi (luca.bondi@polimi.it)
 */

#include "SceneComp.h"

void TileComp::paint(Graphics &g) {
    
    g.setColour(tileColour);
    g.fillPath(path);
    
    g.setColour(Colours::black);
    PathStrokeType strokeType(0.5f);
    g.strokePath(path, strokeType);
}


void TileComp::setColour(const Colour &col) {
    tileColour = col;
}

void TileComp::setPath(const Path& p) {
    path = p;
}
//==============================================================================
//==============================================================================

GridComp::GridComp() {
    // Compute led tresholds
    const float ledStep = 3; //dB
    
    th.clear();
    for (auto ledIdx = ULA_TILE_ROW_COUNT - 1; ledIdx >= 0; --ledIdx) {
        auto ledThDb = ledIdx == (ULA_TILE_ROW_COUNT - 1) ? RED_LT : -((ULA_TILE_ROW_COUNT - 1 - ledIdx) * ledStep);
        th.push_back(ledThDb);
    }
    
}

void GridComp::setCallback(const Callback *c) {
    callback = c;
}

void GridComp::setParams(const std::atomic<float> *config,
                         const std::atomic<float> *frontFacing) {
    frontFacingParam = frontFacing;
    configParam = config;
}

void GridComp::resized() {
    
    if (frontFacingParam != nullptr && configParam != nullptr){
        stopTimer();
        GenericScopedLock<SpinLock> l(lock);
        area = getLocalBounds();
        
        makeLayout();
        
        AffineTransform transf;
        
        if ((bool)(*frontFacingParam)){
            if (isLinearArray(static_cast<MicConfig>((int)*configParam))){
                transf = AffineTransform::rotation(pi, area.getWidth()/2, area.getHeight()/2);
            }else{
                transf = AffineTransform::verticalFlip(area.getHeight()).rotation(pi, area.getWidth()/2, area.getHeight()/2);
            }
        }
        
        for (int rowIdx = 0; rowIdx < tiles.size(); rowIdx++) {
            for (int colIdx = 0; colIdx < tiles[rowIdx].size(); colIdx++) {
                {
                    Path path;
                    path.startNewSubPath(vertices[rowIdx][colIdx]);
                    path.lineTo(vertices[rowIdx + 1][colIdx]);
                    path.lineTo(vertices[rowIdx + 1][colIdx + 1]);
                    path.lineTo(vertices[rowIdx][colIdx + 1]);
                    path.closeSubPath();
                    
                    path.applyTransform(transf);
                    
                    tiles[rowIdx][colIdx]->setPath(path);
                }
                
                Colour baseCol;
                if (isLinearArray(static_cast<MicConfig>((int)*configParam))){
                    baseCol = SingleChannelLedBar::dbToColour(-100,th[rowIdx]);
                }else{
                    baseCol = MultiChannelLedBar::dbToColor(0);
                }
                tiles[rowIdx][colIdx]->setColour(baseCol);
                
                tiles[rowIdx][colIdx]->setBounds(area);
                
            }
        }
        
        if (isLinearArray(static_cast<MicConfig>((int)*configParam))){
            energyPreGain = Mtx(1, NUM_DOAX);
            energy = Mtx(1, NUM_DOAX);
        }else{
            energyPreGain = Mtx(NUM_DOAY, NUM_DOAX);
            energy = Mtx(NUM_DOAY, NUM_DOAX);
        }
        energy.setConstant(-100);
        energyPreGain.setConstant(-100);
        
        startTimerHz(gridUpdateFrequency);
    }
    
}

void GridComp::timerCallback() {
    
    GenericScopedLock<SpinLock> l(lock);
    
    Mtx newEnergy;
    callback->getDoaEnergy(newEnergy);
    
    if (newEnergy.size() != energyPreGain.size())
        return;
    
    energyPreGain = ((1 - inertia) * (newEnergy)) + (inertia * energyPreGain);
    
    // Very basic automatic gain
    auto maxLevel = energyPreGain.maxCoeff() + gain;
    
    if (maxLevel > 3) {
        gain = jmax(gain - 3, minGain);
    } else if (maxLevel < -18) {
        gain = jmin(gain + 1, maxGain);
    } else if (maxLevel > 0) {
        gain = jmax(gain - 0.5f, minGain);
    } else if (maxLevel < -6) {
        gain = jmin(gain + 0.1f, maxGain);
    }
    
    energy = energyPreGain.array() + gain;
    
    for (int rowIdx = 0; rowIdx < tiles.size(); rowIdx++) {
        for (int colIdx = 0; colIdx < tiles[rowIdx].size(); colIdx++) {
            if (configParam != nullptr){
                if (isLinearArray(static_cast<MicConfig>((int)*configParam))){
                    tiles[rowIdx][colIdx]->setColour(SingleChannelLedBar::dbToColour(energy(0,colIdx),th[rowIdx]));
                }else{
                    tiles[rowIdx][colIdx]->setColour(MultiChannelLedBar::dbToColor(energy(rowIdx,colIdx)));
                }
            }
            
        }
    }
    
    repaint();
    
}

void GridComp::makeLayout() {
    
    vertices.resize(0);
    tiles.resize(0);
    
    if (isLinearArray(static_cast<MicConfig>((int)*configParam))){
        vertices.resize(ULA_TILE_ROW_COUNT+1, std::vector<juce::Point<float>>(NUM_DOAX+1));
        
        float angle_diff = MathConstants<float>::pi / NUM_DOAX;
        
        for (int rowIdx = 0; rowIdx <= ULA_TILE_ROW_COUNT; rowIdx++) {
            
            const float radius = jmin(area.getHeight(),area.getWidth()/2) * (1 - (exp((float) rowIdx / ULA_TILE_ROW_COUNT) - 1) / (exp(1) - 1));
            
            for (int colIdx = 0; colIdx <= NUM_DOAX; colIdx++) {
                const float angle = colIdx * angle_diff;
                
                vertices[rowIdx][colIdx].setX(area.getWidth() / 2 - radius * cos(angle));
                vertices[rowIdx][colIdx].setY(area.getHeight() - radius * sin(angle));
                
            }
        }
        
    }else{
        vertices.resize(NUM_DOAY+1, std::vector<juce::Point<float>>(NUM_DOAX+1));
        
        const float deltaY = float(area.getHeight()) / NUM_DOAY;
        const float deltaX = float(area.getWidth()) / NUM_DOAX;
        
        for (int rowIdx = 0; rowIdx <= NUM_DOAY; rowIdx++) {
            for (int colIdx = 0; colIdx <= NUM_DOAX; colIdx++) {
                vertices[rowIdx][colIdx].setY(rowIdx*deltaY);
                vertices[rowIdx][colIdx].setX(colIdx*deltaX);
            }
        }
        
    }
    
    tiles.resize(vertices.size()-1);
    for (auto &tilesRow : tiles){
        tilesRow.resize(NUM_DOAX);
        for (auto &tile :tilesRow){
            tile = std::make_unique<TileComp>();
            addAndMakeVisible(*tile);
        }
    }
    
    
    
}

//==============================================================================
//==============================================================================

void BeamComp::setParams(const std::atomic<float> *config,
                         const std::atomic<float> *frontFacing,
                         const std::atomic<float> *mute,
                         const std::atomic<float> *width,
                         const std::atomic<float> *steerX,
                         const std::atomic<float> *steerY) {
    muteParam = mute;
    widthParam = width;
    steerXParam = steerX;
    steerYParam = steerY;
    frontFacingParam = frontFacing;
    configParam = config;
}

void BeamComp::resized(){
    area = getLocalBounds();
}

void BeamComp::paint(Graphics &g) {

    path.clear();
    
    
    if (isLinearArray(static_cast<MicConfig>((int)*configParam))){
        const float positionX = *steerXParam;
        
        const float width = (0.1 + 2.9 * (*widthParam)) * area.getWidth() / 10;
        path.startNewSubPath(0, 0);
        path.cubicTo(width, -area.getWidth() / 3, width, -area.getWidth() / 2, 0, -area.getWidth() / 2);
        path.cubicTo(-width, -area.getWidth() / 2, -width, -area.getWidth() / 3, 0, 0);
        path.closeSubPath();
        
        path.applyTransform(AffineTransform::rotation((MathConstants<float>::pi / 2) * positionX));
        path.applyTransform(AffineTransform::translation(area.getWidth() / 2, area.getHeight()));
        
        if ((bool) *frontFacingParam) {
            path.applyTransform(AffineTransform::verticalFlip(area.getHeight()));
        }
    }
    else{
        const float positionX = (*steerXParam + 1)*area.getWidth()/2 ;
        const float positionY = area.getHeight() - ((*steerYParam + 1)*area.getHeight()/2);
        const float width = (0.5 + 2.5 * (*widthParam)) * area.getWidth() / 10;
        
        path.startNewSubPath(0, 0);
        path.addEllipse(positionX-width/2, positionY-width/2, width, width);
    }
        

    
    if (!(bool) *muteParam) {
        g.setColour(baseColour.brighter());
        g.setOpacity(0.4);
        g.fillPath(path);
    }
    
    g.setColour(baseColour);
    g.setOpacity(0.9);
    PathStrokeType strokeType(2);
    g.strokePath(path, strokeType);
}

//==============================================================================
//==============================================================================

SceneComp::SceneComp() {
    addAndMakeVisible(grid);
    for (auto &b:beams){
        addAndMakeVisible(b);
        b.addMouseListener(this, true);
    }
}

void SceneComp::setCallback(Callback *c) {
    callback = c;
    grid.setCallback(c);
    grid.setParams(c->getConfigParam(),c->getFrontFacingParam());
    
    for (auto idx = 0; idx < NUM_BEAMS; idx++) {
        beams[idx].setParams(c->getConfigParam(), c->getFrontFacingParam(), c->getBeamMute(idx), c->getBeamWidth(idx), c->getBeamSteerX(idx), c->getBeamSteerY(idx));
    }
    
    resized();
}

void SceneComp::paint(Graphics &g) {
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

void SceneComp::resized() {
    area = getLocalBounds();
    
    if (callback != nullptr)
        if (!isLinearArray(static_cast<MicConfig>((int)*callback->getConfigParam())))
            area.removeFromTop(20);
    
    if (grid.getBounds() == area){
        grid.resized();
    }else{
        grid.setBounds(area);
    }
    
    for (auto &b: beams)
        b.setBounds(area);
}

void SceneComp::setBeamColors(const std::vector<Colour> &colours) {
    jassert(colours.size() == NUM_BEAMS);
    for (auto beamIdx = 0; beamIdx < NUM_BEAMS; ++beamIdx) {
        beams[beamIdx].setBaseColor(colours[beamIdx]);
    }
}

void SceneComp::mouseDown (const MouseEvent& e){
    for (int idx = 0; idx < NUM_BEAMS; idx++){
        if (beams[idx].getPath().contains(e.getMouseDownPosition().toFloat())){
            beamBeingDragged = idx;
            dragStartX = *callback->getBeamSteerX(beamBeingDragged);
            dragStartY = *callback->getBeamSteerY(beamBeingDragged);
            break;
        }
    }
}

void SceneComp::mouseDrag (const MouseEvent& e){
    if (beamBeingDragged >= 0){
        const float deltaX = float(e.getDistanceFromDragStartX())/area.getWidth()*2;
        const float deltaY = float(-e.getDistanceFromDragStartY())/area.getHeight()*2;
        const float newX = jlimit(-1.f,1.f,dragStartX + deltaX);
        const float newY = jlimit(-1.f,1.f,dragStartY + deltaY);
        callback->setBeamSteerX(beamBeingDragged, newX);
        callback->setBeamSteerY(beamBeingDragged, newY);
    }
}

void SceneComp::mouseUp (const MouseEvent& e){
    beamBeingDragged = -1;
}

//==============================================================================
//==============================================================================
