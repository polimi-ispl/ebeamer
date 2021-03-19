/*
 eBeamer Plugin Processor
 
 Authors:
 Luca Bondi (luca.bondi@polimi.it)
 */

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MidiCC.h"
#include "MeterDecay.h"
#include "Beamformer.h"
#include "CpuLoadComp.h"
#include "SceneComp.h"

//==============================================================================

class EbeamerAudioProcessor :
public AudioProcessor,
public AudioProcessorValueTreeState::Listener,
public MeterDecay::Callback,
public CpuLoadComp::Callback,
public SceneComp::Callback,
public MidiCC::Callback,
private OSCReceiver::Listener<juce::OSCReceiver::MessageLoopCallback>
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
    // OSC Callback
    int getOscPort() const;
    bool isOscReady() const;
    
    //==============================================================================
    // MeterDecay Callback
    void getMeterValues(std::vector<float> &meter, int meterId) const override;
    
    float getMeterValue(int meterId, int channel) const override;
    
    //==============================================================================
    // CpuLoadComp Callback
    float getCpuLoad() const override;
    
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
    
    //==============================================================================
    //SceneComponent Callback
    const std::atomic<float> *getConfigParam() const override;
    
    const std::atomic<float> *getFrontFacingParam() const override;
    
    const std::atomic<float> *getBeamMute(int idx) const override;
    
    const std::atomic<float> *getBeamWidth(int idx) const override;
    
    const std::atomic<float> *getBeamSteerX(int idx) const override;
    
    const std::atomic<float> *getBeamSteerY(int idx) const override;
    
    void setBeamSteerX(int idx, float newVal) override;
    
    void setBeamSteerY(int idx, float newVal) override;
    
    void getDoaEnergy(Mtx &energy) const override;
    
    
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
    dsp::Gain<float> beamGain[NUM_BEAMS];
    
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
    void setMicConfig(const MicConfig &mc);
    
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
    
    /** Parameters Tags */
    std::vector<String> paramsTag;
    /** Parameters Types*/
    std::map<String,String> paramsType;
    
    /** Processor parameters tree */
    AudioProcessorValueTreeState parameters;
    
    //==============================================================================
    // VST parameters
    std::atomic<float> *steerBeamXParam[NUM_BEAMS];
    std::atomic<float> *steerBeamYParam[NUM_BEAMS];
    std::atomic<float> *widthBeamParam[NUM_BEAMS];
    std::atomic<float> *panBeamParam[NUM_BEAMS];
    std::atomic<float> *levelBeamParam[NUM_BEAMS];
    std::atomic<float> *muteBeamParam[NUM_BEAMS];
    std::atomic<float> *micGainParam;
    std::atomic<float> *hpfFreqParam;
    std::atomic<float> *frontFacingParam;
    std::atomic<float> *configParam;
    
    void parameterChanged(const String &parameterID, float newValue) override;
    
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
    
    /** UDP socket */
    DatagramSocket socket;
    
    /** OSC receiver instance */
    OSCReceiver oscReceiver;
    
    /** OSC status */
    bool oscReceiverConnected = true;
    
    /** OSC callback */
    void oscMessageReceived (const OSCMessage&) override;
    
    /** Set a state parameter  */
    void setParam(const String&, float);
    void setParam(const String&, bool);
    void setParam(const String&, MicConfig);
    
    /** Message error */
    void showConnectionErrorMessage (const String&);
    
    int oscReceiverPort = 9001;
    
    
    /** Send OSC message */
    void sendOscMessage(OSCSender&, const String&, float) const;
    void sendOscMessage(OSCSender&, const String&, bool) const;
    void sendOscMessage(OSCSender&, const String&, MicConfig) const;
    void sendOscMessage(OSCSender&, const String&, const Mtx&) const;
    void sendOscMessage(OSCSender&, const String&, const std::vector<float>&) const;
    
};
