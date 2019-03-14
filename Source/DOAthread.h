#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

#define INITIAL_CONSIDERED_DIRECTIONS 25
#define BUFFER_THRESHOLD 100

//==============================================================================

class DOAthread     : public Thread
{
public:

    DOAthread(JucebeamAudioProcessor&);
    ~DOAthread();
    
    void run() override;
    std::vector<float> getEnergy();

//==============================================================================
    
private:

    JucebeamAudioProcessor& processor;
    
    SpinLock energyLock;
    
    std::vector<float> energy;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DOAthread);
};