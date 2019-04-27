#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "vAudioParts.h"
#include "vMimoProcessor.h"

#define METERS_DECAY 0.15 //s


//==============================================================================

class JucebeamAudioProcessor  : public AudioProcessor
{
public:

//==============================================================================
    JucebeamAudioProcessor();
    ~JucebeamAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    
    //==============================================================================
    // VST parameters
    AudioParameterFloat* steeringBeam[NUM_BEAMS];
    AudioParameterFloat* widthBeam[NUM_BEAMS];
    AudioParameterFloat* panBeam[NUM_BEAMS];
    AudioParameterFloat* levelBeam[NUM_BEAMS];
    AudioParameterBool*  muteBeam[NUM_BEAMS];
    AudioParameterFloat* micGain;
    AudioParameterFloat* hpfFreq;
    
    //==============================================================================
    // Buffer to allow external access to input signals FFT
    AudioBuffer<float> fftInput;
    bool newFftInputDataAvailable = false;
    SpinLock fftInputLock;
    
    
    //==============================================================================
    // Meters
    float getBeamMeter(int channel);
    std::vector<float> getInputMeters();
    std::vector<float> inputMeters;
    std::vector<float> beamMeters;
    SpinLock inputMetersLock;
    SpinLock beamMetersLock;
    
    //==============================================================================
    // vMimoProcessor
    const std::shared_ptr<vMimoProcessor> getMimoProcessor(){return mimoProcessor;};

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JucebeamAudioProcessor)
    
    //==============================================================================
    // Meters
    std::unique_ptr<vMeterDecay> inputMeterDecay;
    std::unique_ptr<vMeterDecay> beamMeterDecay;
    
    //==============================================================================
    // HPF filters
    float prevHpfFreq = 0;
    IIRCoefficients iirCoeffHPF;
    std::vector<std::unique_ptr<IIRFilter>> iirHPFfilters;

    //==============================================================================
    // Gains
    dsp::Gain<float> commonGain, beamGain[NUM_BEAMS];
    
    //==============================================================================
    // vMimoProcessor
    std::shared_ptr<vMimoProcessor> mimoProcessor;
};
