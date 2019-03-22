/*
  ==============================================================================

    SceneComponent.h
    Created: 15 Mar 2019 5:39:56pm
    Author:  Luca Bondi

  ==============================================================================
*/

#pragma once

#define PLANAR_MODE

#define PERSPECTIVE_RATIO 5

#define TILE_ROW_COUNT 10
#define TILE_COL_COUNT 25
#define PI 3.14159265

#define SCENE_WIDTH 460

#ifdef PLANAR_MODE
#define GUI_HEIGHT 800
#define SCENE_HEIGHT 230
#else
#define GUI_HEIGHT 980
#define SCENE_HEIGHT 460
#endif

#define GRID_REFRESH_TIMER 50

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

//==============================================================================

class TileComponent    : public Component
{
public:
    
    Point<float> corners[2][2];
    Colour tileColour;
    
    TileComponent();
    ~TileComponent();
    
    void paint(Graphics&) override;
    void resized() override;
    
private:
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TileComponent)
};

//==============================================================================

class GridComponent    : public Component, public Timer
{
public:
    GridComponent();
    ~GridComponent();
    
    void resized() override;
    
    void setSource(std::vector<float> &energy, SpinLock &lock)
    {
        this->energy = &energy;
        this->lock = &lock;
        
        startTimer(GRID_REFRESH_TIMER);
    };
    
private:
    
    TileComponent tiles[TILE_ROW_COUNT][TILE_COL_COUNT];
    Point<float> vertices[TILE_ROW_COUNT+1][TILE_COL_COUNT+1];
    
    std::vector<float> *energy;
    SpinLock *lock;
    
    void computeVertices();
    void timerCallback() override;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GridComponent)
};

//==============================================================================

class BeamComponent    : public Component
{
public:
    BeamComponent();
    ~BeamComponent();
    
    void paint(Graphics&) override;
    void resized() override;
    
    void move(float);
    void scale(float);
    
    void setBaseColor(Colour colour){baseColour = colour;}
    
private:
    
    float position;
    float width;
    Colour baseColour = Colours::lightblue;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BeamComponent)
};

//==============================================================================

class SceneComponent    : public Component
{
public:
    SceneComponent();
    ~SceneComponent();
    
    void setBeamColors(const std::vector<Colour> &colours);
    
    void paint(Graphics&) override;
    void resized() override;
    
    GridComponent grid;
    BeamComponent beams[NUM_BEAMS];
    
private:
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SceneComponent)
};
