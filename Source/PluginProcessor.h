#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "AudioParts.h"

#define FFT_SIZE 1024
#define FIR_LEN 512
#define MAX_FFT_BLOCK_LEN (FFT_SIZE - FIR_LEN)
#define NUM_BEAMS 2
#define HPF_FREQ 40.0 //Hz
#define METERS_DECAY 0.15 //s
#define BEAMSTEERING_ALG_IDEAL


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
    
    // Project specific
    std::vector<float*> popFrontFFTdata();
    int bufferStatus();
    SpinLock fftLock;
    
    AudioParameterFloat* steeringBeam[NUM_BEAMS];
    AudioParameterFloat* widthBeam[NUM_BEAMS];
    AudioParameterFloat* panBeam[NUM_BEAMS];
    AudioParameterFloat* gainBeam[NUM_BEAMS];
    AudioParameterBool*  muteBeam[NUM_BEAMS];
    
    std::vector<float> inputMeters;
    std::vector<float> beamMeters;
    SpinLock inputMetersLock;
    SpinLock beamMetersLock;
    
private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JucebeamAudioProcessor)
    
    // Project specific
    std::vector<std::vector<std::vector<float>>> prepareIR(const std::vector<std::vector<std::vector<float>>> fir);
    void prepareForConvolution (float *samples) noexcept;
    void convolutionProcessingAndAccumulate (const float *input, const float *impulse, float *output);
    void updateSymmetricFrequencyDomainData (float* samples) noexcept;
    
    void pushBackFFTdata(float*);
    
    std::vector<std::vector<float*>> fftData;
    
    AudioBuffer<float> beamBuffer;
    std::unique_ptr<dsp::FFT> fft;
    AudioBuffer<float> fftInput;
    AudioBuffer<float> fftBuffer;
    AudioBuffer<float> fftOutput;
    
    dsp::Gain<float> commonGain, beamGain[NUM_BEAMS];
    
    std::vector<std::vector<std::vector<float>>> firBeamwidthFft;
    std::vector<std::vector<std::vector<float>>> firFFT;
    
    IIRCoefficients iirCoeffHPF;
    
    std::unique_ptr<MeterDecay> inputMeterDecay;
    std::unique_ptr<MeterDecay> beamMeterDecay;
    
    std::vector<std::unique_ptr<IIRFilter>> iirHPFfilters;
    
};
