#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#define FFT_SIZE 1024
#define FIR_LEN 512
#define MAX_FFT_BLOCK_LEN (FFT_SIZE - FIR_LEN)
#define NUM_BEAMS 2
#define HPF_FREQ 20.0 //Hz


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
    bool passThrough = false;
    bool bypass = false;
    AudioParameterFloat* steeringBeam[NUM_BEAMS];
    AudioParameterFloat* widthBeam[NUM_BEAMS];
    AudioParameterFloat* panBeam[NUM_BEAMS];
    AudioParameterFloat* gainBeam[NUM_BEAMS];
    AudioParameterBool*  muteBeam[NUM_BEAMS];
    
    typedef enum{UNSPECIFIED,DAS_IDEAL,DAS_MEASURED} algorithmType;
    algorithmType algorithm = DAS_MEASURED;
    
private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JucebeamAudioProcessor)
    
    // Project specific
    std::vector<std::vector<std::vector<float>>> prepareIR(const std::vector<std::vector<std::vector<float>>> fir);
    void prepareForConvolution (float *samples) noexcept;
    void convolutionProcessingAndAccumulate (const float *input, const float *impulse, float *output);
    void updateSymmetricFrequencyDomainData (float* samples) noexcept;
    
    AudioBuffer<float> beamBuffer;
    dsp::FFT *fft;
    float fftInput[2*FFT_SIZE];
    float fftBuffer[2*FFT_SIZE];
    float fftOutput[2*FFT_SIZE];
    
    std::vector<std::vector<std::vector<float>>> firDASidealFft;
    std::vector<std::vector<std::vector<float>>> firDASmeasuredFft;
    std::vector<std::vector<std::vector<float>>> firBeamwidthFft;
    std::vector<std::vector<std::vector<float>>> *firFFT;
    
    IIRCoefficients iirCoeffHPF;
    
    std::vector<IIRFilter*> iirHPFfilters;
    
};
