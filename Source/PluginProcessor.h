#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "AudioParts.h"
#include "Beamformer.h"

//==============================================================================


class EbeamerAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    /** Number of beams */
    static const int numBeams = 2;
    
    /** Number of directions of arrival computed and displayed */
    static const int numDoas = 25;
    
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
    AudioParameterFloat* steeringBeamParam[numBeams];
    AudioParameterFloat* widthBeamParam[numBeams];
    AudioParameterFloat* panBeamParam[numBeams];
    AudioParameterFloat* levelBeamParam[numBeams];
    AudioParameterBool*  muteBeamParam[numBeams];
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
    // Beamformer
    std::unique_ptr<Beamformer>& getBeamformer();
    
    //==============================================================================
    /** Averagel load */
    float getAverageLoad() const;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EbeamerAudioProcessor)
    
    //==============================================================================
    /* Initialize parameters accessible through DAW automation */
    void initializeParameters();
    
    //==============================================================================
    /** Number of active input channels */
    juce::uint32 numActiveInputChannels = 0;
    /** Number of active output channels */
    juce::uint32 numActiveOutputChannels = 0;
    
    //==============================================================================
    /** Time Constant for input gain variations */
    const float gainTimeConst = 0.1;
    /** Input gain, common to all microphones */
    dsp::Gain<float> micGain;
    /** Beam gain for each beam */
    dsp::Gain<float> beamGain[numBeams];
    
    //==============================================================================
    /** Previous HPF cut frequency */
    float prevHpfFreq = 0;
    /** Coefficients of the IIR HPF */
    IIRCoefficients iirCoeffHPF;
    /** IIR HPF */
    std::vector<IIRFilter> iirHPFfilters;
    
    //==============================================================================
    /** The active beamformer */
    std::unique_ptr<Beamformer> beamformer;

    //==============================================================================
    // Meters
    std::unique_ptr<MeterDecay> inputMeterDecay;
    std::unique_ptr<MeterDecay> beamMeterDecay;
    
    /** Decay of  meters [s] */
    const float metersDecay = 0.5;
    
    //==============================================================================
    // Beams buffers
    AudioBuffer<float> beamBuffer;
    
    //==============================================================================
    /** Lock to prevent releaseResources being called when processBlock is running. AudioPluginHost does it. */
    SpinLock processingLock;
    
    /** Resources for runtime are allocated.
     
     This flag is used to compensate for out-of-order calls to prepareToPlay, processBlock and releaseResources
     */
    bool resourcesAllocated = false;
    
    /** Sample rate [Hz] */
    float sampleRate = 48000;
    
    /** Maximum number of samples per block */
    int maximumExpectedSamplesPerBlock = 4096;
    
    //==============================================================================
    
    /** Measured average load */
    float load = 0;
    /** Load update factor (the higher the faster the update) */
    const float loadAlpha = 0.005;
    /** Load lock */
    SpinLock loadLock;
};
