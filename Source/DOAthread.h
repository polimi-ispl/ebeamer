
#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "FIR.h"
#include "MimoProcessor.h"

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
    
    AudioBuffer<float> directionalSignal;
    FIR::AudioBufferFFT inputsFFT;
    std::vector<int> directionIdxs;
    
    double inertia = 0.85;
	double gain = 0;
    const double maxGain = 60, minGain = 0;
    
    IIRCoefficients iirCoeffHPF, iirCoeffLPF;
    std::unique_ptr<IIRFilter> iirHPFfilter, iirLPFfilter;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DOAthread);
    
};
