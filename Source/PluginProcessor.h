#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "AudioParts.h"
#include "Beamformer.h"

//==============================================================================

/** Midi CC type */
typedef struct{
    int channel;
    int number;
} MidiCC;

struct MidiCcCompare{
    bool operator() (const MidiCC& lhs, const MidiCC& rhs) const{
        if (lhs.channel == rhs.channel){
            return lhs.number < rhs.number;
        }
        return lhs.channel < rhs.channel;
    }
};

//==============================================================================


class EbeamerAudioProcessor  : public AudioProcessor,
                               public AudioProcessorValueTreeState::Listener
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
    // Meters
    float getBeamMeter(int channel);
    std::vector<float> getInputMeters();
    std::vector<float> inputMeters;
    std::vector<float> beamMeters;
    SpinLock inputMetersLock;
    SpinLock beamMetersLock;
    
    //==============================================================================
    // Beamformer
    const std::unique_ptr<Beamformer>& getBeamformer() const;
    
    //==============================================================================
    /** Averagel load */
    float getAverageLoad() const;
    
    //==============================================================================
    const AudioProcessorValueTreeState& getParams() const;
    
    //==============================================================================
    /** Start learning the specified parameter */
    void startCCLearning(const String& p);
    /** Stop learning the previous parameter */
    void stopCCLearning();
    /** Get parameter being learned */
    String getCCLearning() const;
    /** Get a read-only reference to the parameters to CC mapping */
    const std::map<String,MidiCC>& getParamToCCMapping();
    /** Remove mapping between MidiCC and parameter */
    void removeCCParamMapping(const String& param);

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EbeamerAudioProcessor)
    
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
    const float metersDecay = 0.2;
    
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
    /** Set a new microphone configuration */
    void setMicConfig(const MicConfig& mc);
    
    //==============================================================================
    
    /** Measured average load */
    float load = 0;
    /** Load update factor (the higher the faster the update) */
    const float loadAlpha = 0.005;
    /** Load lock */
    SpinLock loadLock;
    
    //==============================================================================
    
    /** Processor parameters tree */
    AudioProcessorValueTreeState parameters;
    
    //==============================================================================
    // VST parameters
    std::atomic<float>* steeringBeamParam[numBeams];
    std::atomic<float>* widthBeamParam[numBeams];
    std::atomic<float>* panBeamParam[numBeams];
    std::atomic<float>* levelBeamParam[numBeams];
    std::atomic<float>* muteBeamParam[numBeams];
    std::atomic<float>* micGainParam;
    std::atomic<float>* hpfFreqParam;
    std::atomic<float>* frontFacingParam;
    std::atomic<float>* configParam;
    
    void parameterChanged (const String &parameterID, float newValue) override;
    
    //==============================================================================
    // MIDI management
    
    std::map<MidiCC,String,MidiCcCompare> ccToParamMap;
    std::map<String,MidiCC> paramToCcMap;
    
    /** Process all the received MIDI messages */
    void processMidi(MidiBuffer& midiMessages);
    
    /** Process a MIDI CC message and update parameter as needed */
    void processCC(const MidiCC& cc, int value);
    
    /** Insert mapping between MidiCC and parameter
     
     @return: true if insertion successful, false if either cc or param already mapped
     */
    bool insertCCParamMapping(const MidiCC& cc, const String& param);
    
    /** Parameter whose CC is being learned  */
    String paramCCToLearn = "";
    
};
