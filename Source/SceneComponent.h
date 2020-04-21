/*
  ==============================================================================

    SceneComponent.h
    Created: 15 Mar 2019 5:39:56pm
    Author:  Matteo Scerbo (matteo.scerbo@mail.polimi.it)
    Author:  Luca Bondi (luca.bondi@polimi.it)

  ==============================================================================
*/

#pragma once

#define PERSPECTIVE_RATIO 5
#define TILE_ROW_COUNT 7

#define SCENE_WIDTH 460

#define GUI_HEIGHT 830
#define SCENE_HEIGHT 230


#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "AudioComponents.h"
#include "Beamformer.h"


//==============================================================================

class TileComponent    : public Component
{
public:
    
    juce::Point<float> corners[2][2];
    Colour tileColour;
    
    TileComponent();
    ~TileComponent();
    
    void paint(Graphics&) override;
    void resized() override;
    void setProcessor(const EbeamerAudioProcessor * p);
    
private:
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TileComponent)
    
    const EbeamerAudioProcessor * processor;
};

//==============================================================================

class GridComponent    : public Component, public Timer
{
public:
    GridComponent();
    ~GridComponent();
    
    void resized() override;
    
    void setProcessor(const EbeamerAudioProcessor * p);
    
private:
    
    TileComponent tiles[TILE_ROW_COUNT][EbeamerAudioProcessor::numDoas];
    juce::Point<float> vertices[TILE_ROW_COUNT+1][EbeamerAudioProcessor::numDoas+1];
    
    const EbeamerAudioProcessor * processor;
    
    std::vector<float> th;
    
    std::vector<float> energy, energyPreGain;
    float inertia = 0.85;
    float gain = 0;
    const float maxGain = 20, minGain = -20;
    
    const float gridUpdateFrequency = 10;
    
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
    
    void setProcessor(const EbeamerAudioProcessor * p);
    
    void move(float);
    void scale(float);
    void setStatus(bool);
    
    void setBaseColor(Colour colour){baseColour = colour;}
    
private:
    
    float position;
    float width;
    bool status;
    
    const EbeamerAudioProcessor * processor;
    
    Colour baseColour = Colours::lightblue;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BeamComponent)
};

//==============================================================================

class SceneComponent    : public Component
{
public:
    SceneComponent(const EbeamerAudioProcessor& p);
    ~SceneComponent();
    
    void setBeamColors(const std::vector<Colour> &colours);
    
    void paint(Graphics&) override;
    void resized() override;
    
    GridComponent grid;
    BeamComponent beams[EbeamerAudioProcessor::numBeams];
    
private:
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SceneComponent)
};
