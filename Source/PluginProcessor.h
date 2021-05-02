/*
 eBeamer Plugin Processor
 
 Authors:
 Luca Bondi (luca.bondi@polimi.it)
 */

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MeterDecay.h"
#include "Beamformer.h"

//==============================================================================

class EbeamerAudioProcessor :
public AudioProcessor,
public AudioProcessorValueTreeState::Listener,
public ValueTree::Listener,
public MidiCC::Callback,
public Timer
{
public:
    
    //==============================================================================
    // JUCE plugin methods
    
    EbeamerAudioProcessor();
    
    ~EbeamerAudioProcessor();
    
    const String getName() const override;
    
    bool acceptsMidi() const override;
    
    bool producesMidi() const override;
    
    bool isMidiEffect() const override;
    
    double getTailLengthSeconds() const override;
    
    bool isBusesLayoutSupported(const BusesLayout &layouts) const override;
    
    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override;
    
    void processBlock(AudioBuffer<float> &, MidiBuffer &) override;
    
    void releaseResources() override;
    
    int getNumPrograms() override;
    
    int getCurrentProgram() override;
    
    void setCurrentProgram(int index) override;
    
    const String getProgramName(int index) override;
    
    void changeProgramName(int index, const String &newName) override;
    
    AudioProcessorEditor *createEditor() override;
    
    bool hasEditor() const override;
    
    void getStateInformation(MemoryBlock &destData) override;
    
    void setStateInformation(const void *data, int sizeInBytes) override;
    
    //==============================================================================
    // MidiCC Callback
    /** Start learning the specified parameter */
    void startCCLearning(const String &p) override;
    
    /** Stop learning the previous parameter */
    void stopCCLearning() override;
    
    /** Get parameter being learned */
    String getCCLearning() const override;
    
    /** Get a read-only reference to the parameters to CC mapping */
    const std::map<String, MidiCC> &getParamToCCMapping() override;
    
    /** Remove mapping between MidiCC and parameter */
    void removeCCParamMapping(const String &param) override;

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
    dsp::Gain<float> beamGain[2];
    
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
    const float metersDecay = 0.3;
    
    //==============================================================================
    // Timer to push cpu load, meters, energy updates at a human rate
    void timerCallback() override;
    
    /** Meters update rate [Hz] */
    const float metersUpdateRate = 15;
    
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
    /** Load time constant [s] */
    const float loadTimeConst = 1;
    /** Load update factor (the higher the faster the update) */
    float loadAlpha = 1;
    /** Load lock */
    SpinLock loadLock;
    
    //==============================================================================
        
    /** Processor parameters tree */
    AudioProcessorValueTreeState parameters;
    
    //==============================================================================
    // VST parameters
    std::atomic<float> *steerBeamXParam[2];
    std::atomic<float> *steerBeamYParam[2];
    std::atomic<float> *widthBeamParam[2];
    std::atomic<float> *panBeamParam[2];
    std::atomic<float> *levelBeamParam[2];
    std::atomic<float> *muteBeamParam[2];
    std::atomic<float> *micGainParam;
    std::atomic<float> *hpfFreqParam;
    std::atomic<float> *frontFacingParam;
    std::atomic<float> *configParam;
    
    void parameterChanged(const String &parameterID, float newValue) override;
    
    //==============================================================================
    // Parameters
    ValueTree valueTree;
    void valueTreePropertyChanged (ValueTree &treeWhosePropertyHasChanged, const Identifier &property) override;
    
    void syncParametersToValueTree();
    
    //==============================================================================
    // MIDI management
    
    std::map<MidiCC, String> ccToParamMap;
    std::map<String, MidiCC> paramToCcMap;
    
    /** Process all the received MIDI messages */
    void processMidi(MidiBuffer &midiMessages);
    
    /** Process a MIDI CC message and update parameter as needed */
    void processCC(const MidiCC &cc, int value);
    
    /** Insert mapping between MidiCC and parameter
     
     @return: true if insertion successful, false if either cc or param already mapped
     */
    bool insertCCParamMapping(const MidiCC &cc, const String &param);
    
    /** Parameter whose CC is being learned  */
    String paramCCToLearn = "";
    
    //==============================================================================
    //OSC
    
    /** Set a state parameter  */
    void setParam(const Identifier&, float);
    void setParam(const Identifier&, bool);
    void setParam(const Identifier&, MicConfig);
    
    /** Message error */
    void showConnectionErrorMessage (const String&);
    
    /** OSC controller instance */
    OSCController oscController;
};
