
#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "FIR.h"

//==============================================================================

class DOAthread     : public Thread
{
public:

    DOAthread(EbeamerAudioProcessor& p);
    ~DOAthread();
    
    void run() override;
    
    std::vector<float> energy;
    bool newEnergyAvailable = false;
    SpinLock energyLock;
    
    
//==============================================================================
    
private:
    
    EbeamerAudioProcessor& processor;
    
    double inertia = 0.85;
	double gain = 0;
    const double maxGain = 60, minGain = 0;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DOAthread);
    
};
