#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

#define INITIAL_CONSIDERED_DIRECTIONS 25
#define BUFFER_LOWER_THRESHOLD 5
#define BUFFER_UPPER_THRESHOLD 500

//==============================================================================

class DOAthread     : public Thread
{
public:

    DOAthread(JucebeamAudioProcessor& p);
    ~DOAthread();
    
    void run() override;
    std::vector<float> energy;
    SpinLock energyLock;

//==============================================================================
    
private:

    JucebeamAudioProcessor& processor;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DOAthread);
};
