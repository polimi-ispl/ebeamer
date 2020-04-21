/*
  ==============================================================================

    SceneComponent.cpp
    Created: 15 Mar 2019 5:39:56pm
    Author:  Matteo Scerbo (matteo.scerbo@mail.polimi.it)
    Author:  Luca Bondi (luca.bondi@polimi.it)

  ==============================================================================
*/

#include "SceneComponent.h"

//==============================================================================
//==============================================================================

TileComponent::TileComponent()
{
}

TileComponent::~TileComponent()
{
}

//==============================================================================

void TileComponent::paint(Graphics& g)
{
    Path path;
    
    path.startNewSubPath(corners[0][0]);
    path.lineTo(corners[1][0]);
    path.lineTo(corners[1][1]);
    path.lineTo(corners[0][1]);
    path.closeSubPath();
    
    if (processor->frontFacingParam.getValue()){
        path.applyTransform(AffineTransform::rotation(MathConstants<float>::pi,SCENE_WIDTH/2,SCENE_HEIGHT/2));
    }
    
    g.setColour(tileColour);
    g.fillPath(path);
    
    g.setColour(Colours::black);
    PathStrokeType strokeType(0.5f);
    g.strokePath(path, strokeType);
}

void TileComponent::resized()
{
}

void TileComponent::setProcessor(const EbeamerAudioProcessor * p){
    processor = p;
}
//==============================================================================
//==============================================================================

GridComponent::GridComponent(){
    for(int i = 0; i < TILE_ROW_COUNT; i++)
        for(int j = 0; j < EbeamerAudioProcessor::numDoas; j++)
            addAndMakeVisible (tiles[i][j]);
    
    energy.resize(EbeamerAudioProcessor::numDoas);
    energyPreGain.resize(EbeamerAudioProcessor::numDoas,-30);
        
    
    // Compute led tresholds
    const float ledStep = 3; //dB
    
    th.clear();
    for (auto ledIdx = TILE_ROW_COUNT - 1; ledIdx >= 0; --ledIdx){
        auto ledThDb = ledIdx == (TILE_ROW_COUNT-1) ? RED_LT : -((TILE_ROW_COUNT - 1 - ledIdx) *ledStep);
        th.push_back(ledThDb);
    }
    
    startTimerHz(gridUpdateFrequency);
    
}

GridComponent::~GridComponent()
{
}

void GridComponent::setProcessor(const EbeamerAudioProcessor * p){
    processor = p;
    
    for(int i = 0; i < TILE_ROW_COUNT; i++){
        for(int j = 0; j < EbeamerAudioProcessor::numDoas; j++){
            tiles[i][j].setProcessor(processor);
        }
    }
}

//==============================================================================

void GridComponent::resized()
{
    computeVertices();
    
    for(int i = 0; i < TILE_ROW_COUNT; i++){
        for(int j = 0; j < EbeamerAudioProcessor::numDoas; j++){
            
            tiles[i][j].corners[0][0] = vertices[i  ][j  ];
            tiles[i][j].corners[1][0] = vertices[i+1][j  ];
            tiles[i][j].corners[0][1] = vertices[i  ][j+1];
            tiles[i][j].corners[1][1] = vertices[i+1][j+1];
            
            tiles[i][j].setBounds(getLocalBounds());
            
            if(i < TILE_ROW_COUNT/4)
                tiles[i][j].tileColour = Colours::red.darker(0.9);
            
            if(TILE_ROW_COUNT/4 <= i && i < TILE_ROW_COUNT/2)
                tiles[i][j].tileColour = Colours::yellow.darker(0.9);
            
            if(i >= TILE_ROW_COUNT/2)
                tiles[i][j].tileColour = Colours::green.darker(0.9);
            
        }
    }
}

void GridComponent::timerCallback(){
    
    std::vector<float> newEnergy(EbeamerAudioProcessor::numDoas);
    processor->getBeamformer()->getDoaEnergy(newEnergy);
    
    for (auto dirIdx = 0; dirIdx < energyPreGain.size(); ++dirIdx){
        energyPreGain[dirIdx] = ((1-inertia) * (newEnergy[dirIdx])) + (inertia * energyPreGain[dirIdx]);
    }
    
    // Very basic automatic gain
    auto rangeEnergy = FloatVectorOperations::findMinAndMax(energyPreGain.data(), (int)energyPreGain.size());
    auto minLevel = rangeEnergy.getStart() + gain;
    auto maxLevel = rangeEnergy.getEnd() + gain;
    
    if (maxLevel > 0){
        gain = jmax(gain-maxLevel-3,minGain);
    }else if (maxLevel < -18){
        gain = jmin(gain-maxLevel,maxGain);
    }else if (maxLevel > -3){
        gain = jmax(gain-0.5f,minGain);
    }else if (maxLevel < -9){
        gain = jmin(gain+0.5f,maxGain);
    }
    
    for (auto dirIdx = 0; dirIdx < energyPreGain.size(); ++dirIdx){
        energy[dirIdx] = energyPreGain[dirIdx] + gain;
    }

    for(int j = 0; j < EbeamerAudioProcessor::numDoas; j++){
        for(int i = 0; i < TILE_ROW_COUNT; i++){
            tiles[i][j].tileColour = SingleChannelLedBar::thToColour(th[i],energy[j] > th[i]);
        }
    }
    
    repaint();
    
}

void GridComponent::computeVertices()
{
    const float w = SCENE_WIDTH;
    const float h = SCENE_HEIGHT;
    
    float angle_diff = MathConstants<float>::pi / EbeamerAudioProcessor::numDoas;
    // float radius_diff = h / TILE_ROW_COUNT;
    
    for(int i = 0; i <= TILE_ROW_COUNT; i++){
        // Linear
        // float radius = h - i * radius_diff;
        
        // Square
        // float radius = h - h * pow( (float)i / TILE_ROW_COUNT, 2 );
        
        // Inverse square
        // float radius = h * sqrt( (float)(TILE_ROW_COUNT - i) / TILE_ROW_COUNT );
        
        // Exponential
        const float radius = h - h * ( exp( (float)i / TILE_ROW_COUNT ) - 1 ) / ( exp( 1 ) - 1 );
        
        for(int j = 0; j <= EbeamerAudioProcessor::numDoas; j++){
            const float angle = j*angle_diff;
            
            vertices[i][j].setX( w/2 - radius*cos(angle));
            vertices[i][j].setY( h  - radius*sin(angle));

        }
    }
}

//==============================================================================
//==============================================================================

BeamComponent::BeamComponent(){
    position = 0;
}

BeamComponent::~BeamComponent(){
}

void BeamComponent::setProcessor(const EbeamerAudioProcessor * p){
    processor = p;
}

//==============================================================================

void BeamComponent::paint(Graphics& g){
    Path path;
    
    path.startNewSubPath(0, 0);
    path.cubicTo( width, -SCENE_WIDTH/3,  width, -SCENE_WIDTH/2, 0, -SCENE_WIDTH/2);
    path.cubicTo(-width, -SCENE_WIDTH/2, -width, -SCENE_WIDTH/3, 0, 0);
    path.closeSubPath();
    
    path.applyTransform(AffineTransform::rotation( (MathConstants<float>::pi/2) * position));
    path.applyTransform(AffineTransform::translation(SCENE_WIDTH/2, SCENE_WIDTH/2));
    
    if (processor->frontFacingParam.getValue()){
        //path.applyTransform(AffineTransform::rotation(MathConstants<float>::pi,SCENE_WIDTH/2,SCENE_HEIGHT/2));
        path.applyTransform(AffineTransform::verticalFlip(SCENE_HEIGHT));
    }
    
    if (status){
        g.setColour(baseColour.brighter());
        g.setOpacity(0.4);
        g.fillPath(path);
    }
    
    g.setColour (baseColour);
    g.setOpacity(0.8);
    PathStrokeType strokeType(2);
    g.strokePath(path, strokeType);
}

void BeamComponent::resized(){
}

void BeamComponent::move(float new_position){
    position = new_position;
    repaint();
}

void BeamComponent::scale(float new_width){
    width = (0.1 + 2.9*new_width) * SCENE_WIDTH/10;
    repaint();
}

void BeamComponent::setStatus(bool s){
    status = s;
    repaint();
}

//==============================================================================
//==============================================================================

SceneComponent::SceneComponent(const EbeamerAudioProcessor& p){
    
    
    beams[0].move(-0.5);
    beams[0].scale(0.25);
    beams[0].setProcessor(&p);
    
    beams[1].move(0.5);
    beams[1].scale(0.5);
    beams[1].setProcessor(&p);
    
    grid.setProcessor(&p);
    
    addAndMakeVisible (grid);
    for(int i = 0; i < EbeamerAudioProcessor::numBeams; i++)
        addAndMakeVisible (beams[i]);
    
}

SceneComponent::~SceneComponent()
{
}

//==============================================================================

void SceneComponent::paint(Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
}

void SceneComponent::resized()
{
    grid.setBounds(getLocalBounds());
    for(int i = 0; i < EbeamerAudioProcessor::numBeams; i++)
        beams[i].setBounds(getLocalBounds());
}

void SceneComponent::setBeamColors(const std::vector<Colour> &colours){
    jassert(colours.size() == EbeamerAudioProcessor::numBeams);
    for (auto beamIdx = 0;beamIdx < EbeamerAudioProcessor::numBeams;++beamIdx)
    {
        beams[beamIdx].setBaseColor(colours[beamIdx]);
    }
}

//==============================================================================
//==============================================================================
