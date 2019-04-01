/*
  ==============================================================================

    SceneComponent.cpp
    Created: 15 Mar 2019 5:39:56pm
    Author:  Luca Bondi

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
    
    g.setColour(tileColour);
    g.fillPath(path);
    
    g.setColour(Colours::black);
    PathStrokeType strokeType(0.5f);
    g.strokePath(path, strokeType);
}

void TileComponent::resized()
{
}

//==============================================================================
//==============================================================================

GridComponent::GridComponent()
{
    for(int i = 0; i < TILE_ROW_COUNT; i++)
        for(int j = 0; j < TILE_COL_COUNT; j++)
            addAndMakeVisible (tiles[i][j]);
    
    computeVertices();
    
    // Compute led tresholds
    const float ledStep = 3; //dB
    
    th.clear();
    for (auto ledIdx = TILE_ROW_COUNT - 1; ledIdx >= 0; --ledIdx)
    {
        auto ledThDb = ledIdx == (TILE_ROW_COUNT-1) ? RED_LT : -((TILE_ROW_COUNT - 1 - ledIdx) *ledStep);
        th.push_back(ledThDb);
    }
    
}

GridComponent::~GridComponent()
{
}

//==============================================================================

void GridComponent::resized()
{
    computeVertices();
    
    for(int i = 0; i < TILE_ROW_COUNT; i++){
        for(int j = 0; j < TILE_COL_COUNT; j++){
            
            tiles[i][j].corners[0][0] = vertices[i  ][j  ];
            tiles[i][j].corners[1][0] = vertices[i+1][j  ];
            tiles[i][j].corners[0][1] = vertices[i  ][j+1];
            tiles[i][j].corners[1][1] = vertices[i+1][j+1];
            
            tiles[i][j].setBounds(getLocalBounds());
            
#ifdef PLANAR_MODE
            // 2D grid
            
            if(i < TILE_ROW_COUNT/4)
                tiles[i][j].tileColour = Colours::red.darker(0.9);
            
            if(TILE_ROW_COUNT/4 <= i && i < TILE_ROW_COUNT/2)
                tiles[i][j].tileColour = Colours::yellow.darker(0.9);
            
            if(i >= TILE_ROW_COUNT/2)
                tiles[i][j].tileColour = Colours::green.darker(0.9);
            
#else
            // 3D grid
            
            if( (i + j)% 2 == 0 ){
                if( i == floor(TILE_ROW_COUNT/2) )
                    tiles[i][j].tileColour = Colours::white;
                else
                    tiles[i][j].tileColour = Colours::grey;
            }
            
#endif
        }
    }
}

void GridComponent::timerCallback()
{
    // TODO: keep track of # of directions (DOAthread might change it for performance)
    // and re-compute the grid if necessary.
    // I tried changing tiles and vertices from arrays to vectors but it didn't work
    // because of component initializations during vector resizing.
    /*
     if(energy.size() != consideredDirections){
     consideredDirections = energy.size();
     computeVertices();
     }
     */
    
    std::vector<float> energy; // dB
    {
        if (doaThread->newEnergyAvailable == false){
            return;
        }
        GenericScopedLock<SpinLock> lock(doaThread->energyLock);
        energy = doaThread->energy;
        doaThread->newEnergyAvailable = false;
    }
    
    if(energy.size() != TILE_COL_COUNT){
        return;
    }
    
    for(int j = 0; j < TILE_COL_COUNT; j++){
        
        for(int i = 0; i < TILE_ROW_COUNT; i++){
            
#ifdef PLANAR_MODE
            // 2D grid
            
            tiles[i][j].tileColour = SingleChannelLedBar::thToColour(th[i],energy[j] > th[i]);
            
#else
            // 3D grid
            
#endif
        }
    }
    
    {
        //MessageManagerLock mmlock;
        repaint();
    }
    
}

void GridComponent::computeVertices()
{
#ifdef PLANAR_MODE
    // 2D grid
    
    float w = SCENE_WIDTH;
    float h = w/2;
    
    float angle_diff = MathConstants<float>::pi / TILE_COL_COUNT;
    float radius_diff = h / TILE_ROW_COUNT;
    
    for(int i = 0; i <= TILE_ROW_COUNT; i++){
        // Linear
        // float radius = h - i * radius_diff;
        
        // Square
        // float radius = h - h * pow( (float)i / TILE_ROW_COUNT, 2 );
        
        // Inverse square
        // float radius = h * sqrt( (float)(TILE_ROW_COUNT - i) / TILE_ROW_COUNT );
        
        // Exponential
        float radius = h - h * ( exp( (float)i / TILE_ROW_COUNT ) - 1 ) / ( exp( 1 ) - 1 );
        
        for(int j = 0; j <= TILE_COL_COUNT; j++){
            float angle = j*angle_diff;
            
            vertices[i][j].setX( w/2 - radius*cos(angle));
            vertices[i][j].setY( h  - radius*sin(angle));
        }
    }
    
#else
    // 3D grid
    
    float w = SCENE_WIDTH;
    float h = w;
    float radius = w/2;
    float pov = radius + radius * PERSPECTIVE_RATIO;
    
    float R_angle_diff = PI / TILE_ROW_COUNT;
    float C_angle_diff = PI / TILE_COL_COUNT;
    
    for(int i = 0; i <= TILE_ROW_COUNT; i++){
        float R_angle = i * R_angle_diff;
        
        for(int j = 0; j <= TILE_COL_COUNT; j++){
            float C_angle = PI - (j * C_angle_diff);
            
            vertices[i][j].setX(
                                radius +
                                ( pov * ( radius * cos(C_angle) * sin(R_angle) ) )
                                /
                                ( pov + ( radius * sin(C_angle) * sin(R_angle) ) )
                                );
            
            vertices[i][j].setY(
                                radius -
                                ( pov * ( radius * cos(R_angle) ) )
                                /
                                ( pov + ( radius * sin(C_angle) * sin(R_angle) ) )
                                );
        }
    }
    
#endif
}

//==============================================================================
//==============================================================================

BeamComponent::BeamComponent()
{
    position = 0;
}

BeamComponent::~BeamComponent()
{
}

//==============================================================================

void BeamComponent::paint(Graphics& g)
{
#ifdef PLANAR_MODE
    
    Path path;
    
    path.startNewSubPath(0, 0);
    path.cubicTo( width, -SCENE_WIDTH/3,  width, -SCENE_WIDTH/2, 0, -SCENE_WIDTH/2);
    path.cubicTo(-width, -SCENE_WIDTH/2, -width, -SCENE_WIDTH/3, 0, 0);
    path.closeSubPath();
    
    path.applyTransform(AffineTransform::rotation( (MathConstants<float>::pi/2) * position));
    path.applyTransform(AffineTransform::translation(SCENE_WIDTH/2, SCENE_WIDTH/2));
    
    g.setColour(baseColour.brighter());
    g.setOpacity(0.4);
    g.fillPath(path);
    
    g.setColour (baseColour);
    g.setOpacity(0.8);
    PathStrokeType strokeType(2);
    g.strokePath(path, strokeType);
    
#else
    
#endif
}

void BeamComponent::resized()
{
}

void BeamComponent::move(float new_position)
{
    position = new_position;
    repaint();
}

void BeamComponent::scale(float new_width)
{
    width = (0.1 + 2.9*new_width) * SCENE_WIDTH/10;
    repaint();
}

//==============================================================================
//==============================================================================

SceneComponent::SceneComponent()
{
    addAndMakeVisible (grid);
    for(int i = 0; i < NUM_BEAMS; i++)
        addAndMakeVisible (beams[i]);
    
    beams[0].move(-0.5);
    beams[0].scale(0.25);
    
    beams[1].move(0.5);
    beams[1].scale(0.5);
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
    for(int i = 0; i < NUM_BEAMS; i++)
        beams[i].setBounds(getLocalBounds());
}

void SceneComponent::setBeamColors(const std::vector<Colour> &colours){
    jassert(colours.size() == NUM_BEAMS);
    for (auto beamIdx = 0;beamIdx < NUM_BEAMS;++beamIdx)
    {
        beams[beamIdx].setBaseColor(colours[beamIdx]);
    }
}

//==============================================================================
//==============================================================================
