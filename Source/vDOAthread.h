#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "vFIR.h"

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
    
    double inertia = 0.85;
	double gain = 0;
    const double maxGain = 60, minGain = 0;
    
    IIRCoefficients iirCoeffHPF, iirCoeffLPF;
    std::unique_ptr<IIRFilter> iirHPFfilter, iirLPFfilter;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DOAthread);
    
};
