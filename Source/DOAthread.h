#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

#define INITIAL_CONSIDERED_DIRECTIONS 25
#define BUFFER_LOWER_THRESHOLD 5
#define BUFFER_UPPER_THRESHOLD 50

//==============================================================================

class DOAthread     : public Thread
{
public:

    DOAthread(JucebeamAudioProcessor&);
    ~DOAthread();
    
    void run() override;
    std::vector<float> energy;

//==============================================================================
    
private:

    JucebeamAudioProcessor& processor;
    
    SpinLock energyLock;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DOAthread);
};
