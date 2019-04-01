#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

//==============================================================================

class DOAthread     : public Thread
{
public:

    DOAthread(JucebeamAudioProcessor& p);
    ~DOAthread();
    
    void run() override;
    
    std::vector<float> energy;
    bool newEnergyAvailable = false;
    SpinLock energyLock;
    
    
//==============================================================================
    
private:
    
    std::unique_ptr<dsp::FFT> fft;
    
    JucebeamAudioProcessor& processor;
    
    AudioBuffer<float> directionalSignal;
    AudioBuffer<float> fftOutput;
    
    AudioBuffer<float> fftInput;
    
    std::vector<int> directionIdxs;
    
    float inertia = 0.9;
    float gain = 18;
    
    IIRCoefficients iirCoeffHPF;
    std::unique_ptr<IIRFilter> iirHPFfilter;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DOAthread);
    
};
