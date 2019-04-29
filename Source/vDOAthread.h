#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "vFIR.h"
#include "vMimoProcessor.h"

//==============================================================================

class vDOAthread     : public Thread
{
public:

    vDOAthread(JucebeamAudioProcessor& p);
    ~vDOAthread();
    
    void run() override;
    
    std::vector<float> energy;
    bool newEnergyAvailable = false;
    SpinLock energyLock;
    
    
//==============================================================================
    
private:
    
    JucebeamAudioProcessor& processor;
    
    AudioBuffer<float> directionalSignal;
    vFIR::AudioBufferFFT inputsFFT;
    std::vector<int> directionIdxs;
    
    double inertia = 0.85;
	double gain = 0;
    const double maxGain = 60, minGain = 0;
    
    IIRCoefficients iirCoeffHPF, iirCoeffLPF;
    std::unique_ptr<IIRFilter> iirHPFfilter, iirLPFfilter;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (vDOAthread);
    
};
