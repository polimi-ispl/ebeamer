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

#define TILE_ROW_COUNT 7
#define TILE_COL_COUNT 25

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
#include "vAudioComponents.h"
#include "vDOAthread.h"

//==============================================================================

class vTileComponent    : public Component
{
public:
    
    Point<float> corners[2][2];
    Colour tileColour;
    
    vTileComponent();
    ~vTileComponent();
    
    void paint(Graphics&) override;
    void resized() override;
    
private:
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (vTileComponent)
};

//==============================================================================

class vGridComponent    : public Component, public Timer
{
public:
    vGridComponent();
    ~vGridComponent();
    
    void resized() override;
    
    void setSource(std::shared_ptr<vDOAthread> d){doaThread = d;};
    
private:
    
    vTileComponent tiles[TILE_ROW_COUNT][TILE_COL_COUNT];
    Point<float> vertices[TILE_ROW_COUNT+1][TILE_COL_COUNT+1];
    
    std::shared_ptr<vDOAthread> doaThread;
    
    std::vector<float> th;
    
    void computeVertices();
    void timerCallback() override;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (vGridComponent)
};

//==============================================================================

class vBeamComponent    : public Component
{
public:
    vBeamComponent();
    ~vBeamComponent();
    
    void paint(Graphics&) override;
    void resized() override;
    
    void move(float);
    void scale(float);
    void setStatus(bool);
    
    void setBaseColor(Colour colour){baseColour = colour;}
    
private:
    
    float position;
    float width;
    bool status;
    
    Colour baseColour = Colours::lightblue;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (vBeamComponent)
};

//==============================================================================

class vSceneComponent    : public Component
{
public:
    vSceneComponent();
    ~vSceneComponent();
    
    void setBeamColors(const std::vector<Colour> &colours);
    
    void paint(Graphics&) override;
    void resized() override;
    
    vGridComponent grid;
    vBeamComponent beams[NUM_BEAMS];
    
private:
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (vSceneComponent)
};
