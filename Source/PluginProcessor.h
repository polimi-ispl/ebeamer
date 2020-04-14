#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "AudioParts.h"
#include "MimoProcessor.h"
#define NUM_BEAMS 2
#define OUT_CHANNELS 2
#define METERS_DECAY 0.15 //s


//==============================================================================

class EbeamerAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    EbeamerAudioProcessor();
    ~EbeamerAudioProcessor();
    
    const String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void prepareToPlay (double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;
    void releaseResources() override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    //==============================================================================
    // VST parameters
    AudioParameterFloat* steeringBeamParam[NUM_BEAMS];
    AudioParameterFloat* widthBeamParam[NUM_BEAMS];
    AudioParameterFloat* panBeamParam[NUM_BEAMS];
    AudioParameterFloat* levelBeamParam[NUM_BEAMS];
    AudioParameterBool*  muteBeamParam[NUM_BEAMS];
    AudioParameterFloat* micGainParam;
    AudioParameterFloat* hpfFreqParam;
    
    //==============================================================================
    // Buffer to allow external access to input signals FFT
    FIR::AudioBufferFFT inputsFFT;
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
    const std::shared_ptr<MimoProcessor> getMimoProcessor(){return mimoProcessor;};

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EbeamerAudioProcessor)
    
    /* Initialize parameters accessible through DAW automation */
    void initializeParameters();
    
    //==============================================================================
    // Meters
    std::unique_ptr<MeterDecay> inputMeterDecay;
    std::unique_ptr<MeterDecay> beamMeterDecay;
    
    //==============================================================================
    // HPF filters
    float prevHpfFreq = 0;
    IIRCoefficients iirCoeffHPF;
    std::vector<std::unique_ptr<IIRFilter>> iirHPFfilters;

    //==============================================================================
    // Gains
    /** Time Constant for input gain variations */
    const float inputGainTimeConst = 0.1;
    dsp::Gain<float> micGain, beamGain[NUM_BEAMS];
    
    //==============================================================================
    // MimoProcessor
    std::shared_ptr<MimoProcessor> mimoProcessor;
    size_t numSteeringDirections;
    size_t numBeamwidthChoices;
    
    //==============================================================================
    // Beams buffers
    AudioBuffer<float> beamsBuffer;
    
    //==============================================================================
    
    /* Number of active input channels */
    juce::uint32 numActiveInputChannels = 0;
    const juce::uint32 numActiveOutputChannels = 2;
    
    
};
