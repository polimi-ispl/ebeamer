/*
 eBeamer Plugin Processor
 
 Authors:
 Luca Bondi (luca.bondi@polimi.it)
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

// Helper functions
AudioProcessorValueTreeState::ParameterLayout initializeParameters() {
    
    
    std::vector<std::unique_ptr<RangedAudioParameter>> params;
    
    // Values in dB
    params.push_back(std::make_unique<AudioParameterChoice>(configIdentifier.toString(), //tag
                                                            "Configuration", //name
                                                            micConfigLabels, //choices
                                                            0 //default
                                                            ));
    
    params.push_back(std::make_unique<AudioParameterBool>(frontIdentifier.toString(), //tag
                                                          "Front facing", //name
                                                          false //default
                                                          ));
    
    params.push_back(std::make_unique<AudioParameterFloat>(gainIdentifier.toString(), //tag
                                                           "Mic gain", //name
                                                           MIN_GAIN, //min
                                                           MAX_GAIN, //max
                                                           DEFAULT_GAIN //default
                                                           ));
    
    // Values in Hz
    params.push_back(std::make_unique<AudioParameterFloat>(hpfIdentifier.toString(), //tag
                                                           "HPF",
                                                           MIN_HPF, //min
                                                           MAX_HPF, //max
                                                           DEFAULT_HPF //default
                                                           ));
    
    {
        for (auto beamIdx = 0; beamIdx < 2; ++beamIdx) {
            auto defaultDirectionX = beamIdx == 0 ? -0.5 : 0.5;
            params.push_back(std::make_unique<AudioParameterFloat>(steerXIdentifierPrefix + String(beamIdx + 1), //tag
                                                                   "Steer " + String(beamIdx + 1) + " hor", //name
                                                                   -1.0f, //min
                                                                   1.0f, //max
                                                                   defaultDirectionX //default
                                                                   ));
            
            params.push_back(std::make_unique<AudioParameterFloat>(steerYIdentifierPrefix + String(beamIdx + 1), //tag
                                                                   "Steer " + String(beamIdx + 1) + " ver", //name
                                                                   -1.0f, //min
                                                                   1.0f, //max
                                                                   0 //default
                                                                   ));
            
            params.push_back(std::make_unique<AudioParameterFloat>(widthIdentifierPrefix + String(beamIdx + 1), //tag
                                                                   "Width beam" + String(beamIdx + 1), //name
                                                                   0.0f, //min
                                                                   1.0f,//max
                                                                   0.3f//default
                                                                   ));
            
            auto defaultPan = beamIdx == 0 ? -0.5 : 0.5;
            params.push_back(std::make_unique<AudioParameterFloat>(panIdentifierPrefix + String(beamIdx + 1), //tag
                                                                   "Pan beam" + String(beamIdx + 1), //name
                                                                   -1.0f, //min
                                                                   1.0f, //max
                                                                   defaultPan //default
                                                                   ));
            
            params.push_back(std::make_unique<AudioParameterFloat>(levelIdentifierPrefix + String(beamIdx + 1), //tag
                                                                   "Level beam" + String(beamIdx + 1), //name
                                                                   MIN_LEVEL, //min
                                                                   MAX_LEVEL, //max
                                                                   0.0f //default
                                                                   ));
            
            params.push_back(std::make_unique<AudioParameterBool>(muteIdentifierPrefix + String(beamIdx + 1), //tag
                                                                  "Mute beam" + String(beamIdx + 1), //name
                                                                  false //default
                                                                  ));
            
        }
    }
    
    return {params.begin(), params.end()};
}


//==============================================================================
//The default bus layout accommodates for 4 buses of 16 channels each for VST3 mode, one bus with 64 channels for standalone mode.
EbeamerAudioProcessor::EbeamerAudioProcessor()
: AudioProcessor(JUCEApplication::isStandaloneApp()
                 ?
                 BusesProperties()
                 .withInput("eSticks", AudioChannelSet::channelSetsWithNumberOfChannels(64)[0])
                 .withOutput("Output", AudioChannelSet::stereo(), true)
                 :
                 BusesProperties()
                 .withInput("eStick#1", AudioChannelSet::ambisonic(3), true)
                 .withInput("eStick#2", AudioChannelSet::ambisonic(3), true)
                 .withInput("eStick#3", AudioChannelSet::ambisonic(3), true)
                 .withInput("eStick#4", AudioChannelSet::ambisonic(3), true)
                 .withOutput("Output", AudioChannelSet::stereo(), true)
                 ),
parameters(*this,
           nullptr,
           Identifier("eBeamerParams"),
           initializeParameters()
           )
{
    
    /** Setup parameters listener and pointers */
    configParam = parameters.getRawParameterValue(configIdentifier.toString());
    frontFacingParam = parameters.getRawParameterValue(frontIdentifier.toString());
    hpfFreqParam = parameters.getRawParameterValue(hpfIdentifier.toString());
    micGainParam = parameters.getRawParameterValue(gainIdentifier.toString());
    
    parameters.addParameterListener(configIdentifier.toString(), this);
    parameters.addParameterListener(frontIdentifier.toString(), this);
    parameters.addParameterListener(hpfIdentifier.toString(), this);
    parameters.addParameterListener(gainIdentifier.toString(), this);
    
    for (auto beamIdx = 0; beamIdx < 2; beamIdx++) {
        steerBeamXParam[beamIdx] = parameters.getRawParameterValue(steerXIdentifierPrefix + String(beamIdx + 1));
        steerBeamYParam[beamIdx] = parameters.getRawParameterValue(steerYIdentifierPrefix + String(beamIdx + 1));
        widthBeamParam[beamIdx] = parameters.getRawParameterValue(widthIdentifierPrefix + String(beamIdx + 1));
        panBeamParam[beamIdx] = parameters.getRawParameterValue(panIdentifierPrefix + String(beamIdx + 1));
        levelBeamParam[beamIdx] = parameters.getRawParameterValue(levelIdentifierPrefix + String(beamIdx + 1));
        muteBeamParam[beamIdx] = parameters.getRawParameterValue(muteIdentifierPrefix + String(beamIdx + 1));
        
        parameters.addParameterListener(steerXIdentifierPrefix + String(beamIdx + 1), this);
        parameters.addParameterListener(steerYIdentifierPrefix + String(beamIdx + 1), this);
        parameters.addParameterListener(widthIdentifierPrefix + String(beamIdx + 1), this);
        parameters.addParameterListener(panIdentifierPrefix + String(beamIdx + 1), this);
        parameters.addParameterListener(levelIdentifierPrefix + String(beamIdx + 1), this);
        parameters.addParameterListener(muteIdentifierPrefix + String(beamIdx + 1), this);
    }
    
    /* Value tree for session parameters */
    valueTree = ValueTree("Ebeamer");
    initValueTreeParameters(valueTree);
    valueTree.setProperty(serverPortIdentifier, 0, nullptr);
    
    syncParametersToValueTree();
    
    /** Listen to valueTree to keep in sync with parameters */
    valueTree.addListener(this);
    
    //==============================================================================
    /* OSC controller */
    oscController.init(valueTree);
    registerCommonOscParameters(oscController);
    if (oscController.startReceiver()){
        valueTree.setProperty(serverPortIdentifier, oscController.getReceiverPort(), nullptr);
        oscController.startBroadcast();
    }else{
        std::ostringstream errMsg;
        errMsg << "Error: cannot initialize OSC receiver";
        showConnectionErrorMessage (errMsg.str());
    }
    
}

//==============================================================================
bool EbeamerAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const {
    if (!JUCEApplication::isStandaloneApp()){
        // This plug-in supports up to 4 eSticks, for a total amount of 64 channels in input.
        // VST3 allows for a maximum of 25 channels per bus.
        // To make things simpler in terms of patching, for VST each input bus counts for at most 16 channels.
        // This configuration allows REAPER to be configured with a 64 channels track.
        
        for (auto bus : layouts.inputBuses) {
            if (bus.size() > 16) {
                return false;
            }
        }
        for (auto bus : layouts.outputBuses) {
            if (bus.size() > 16) {
                // We have to allow the output bus to grow to the size of the input bus for compatibility with REAPER
                return false;
            }
        }
    }
    
    if ((layouts.getMainInputChannels() < 1) || (layouts.getMainOutputChannels() < 2)) {
        // In any case don't allow less than 1 input and 2 output channels
        return false;
    }
    return true;
}

//==============================================================================
void EbeamerAudioProcessor::prepareToPlay(double sampleRate_, int maximumExpectedSamplesPerBlock_) {
    
    GenericScopedLock<SpinLock> lock(processingLock);
    
    stopTimer();
    
    sampleRate = sampleRate_;
    maximumExpectedSamplesPerBlock = maximumExpectedSamplesPerBlock_;
    
    /** Number of active input channels */
    numActiveInputChannels = getTotalNumInputChannels();
    
    /** Number of active output channels */
    numActiveOutputChannels = jmin(2, getTotalNumOutputChannels());
    
    /** Initialize the input gain */
    micGain.reset();
    micGain.prepare({sampleRate, static_cast<uint32>(maximumExpectedSamplesPerBlock), numActiveInputChannels});
    micGain.setGainDecibels(*micGainParam);
    micGain.setRampDurationSeconds(gainTimeConst);
    
    /** Initialize the High Pass Filters */
    iirHPFfilters.clear();
    iirHPFfilters.resize(numActiveInputChannels);
    prevHpfFreq = 0;
    
    /** Initialize the beamformer */
    beamformer = std::make_unique<Beamformer>(2, static_cast<MicConfig>((int) *configParam),sampleRate, maximumExpectedSamplesPerBlock, metersUpdateRate);
    
    /** Initialize beams' buffer  */
    beamBuffer.setSize(2, maximumExpectedSamplesPerBlock);
    
    /** Initialize beam level gains */
    for (auto beamIdx = 0; beamIdx < 2; ++beamIdx) {
        beamGain[beamIdx].reset();
        beamGain[beamIdx].prepare({sampleRate, static_cast<uint32>(maximumExpectedSamplesPerBlock), 1});
        beamGain[beamIdx].setGainDecibels(*levelBeamParam[beamIdx]);
        beamGain[beamIdx].setRampDurationSeconds(gainTimeConst);
    }
    
    /** initialize meters */
    inputMeterDecay = std::make_unique<MeterDecay>(sampleRate, metersDecay, maximumExpectedSamplesPerBlock,
                                                   numActiveInputChannels);
    beamMeterDecay = std::make_unique<MeterDecay>(sampleRate, metersDecay, maximumExpectedSamplesPerBlock, 2);
    
    resourcesAllocated = true;
    
    /** Time constants */
    loadAlpha = 1 - exp(-(maximumExpectedSamplesPerBlock / sampleRate) / loadTimeConst);
    
    startTimerHz(metersUpdateRate);
}

void EbeamerAudioProcessor::releaseResources() {
    
    GenericScopedLock<SpinLock> lock(processingLock);
    
    resourcesAllocated = false;
    
    /** Clear beam buffer */
    beamBuffer.setSize(2, 0);
    
    /** Clear the HPF */
    iirHPFfilters.clear();
    
    /** Clear the Beamformer */
    beamformer.reset();
}

bool EbeamerAudioProcessor::insertCCParamMapping(const MidiCC &cc, const String &param) {
    if (paramToCcMap.count(param) > 0 || ccToParamMap.count(cc) > 0) {
        return false;
    }
    ccToParamMap[cc] = param;
    paramToCcMap[param] = cc;
    return true;
}

void EbeamerAudioProcessor::removeCCParamMapping(const String &param) {
    if (paramToCcMap.count(param) > 0) {
        auto cc = paramToCcMap[param];
        paramToCcMap.erase(param);
        ccToParamMap.erase(cc);
    }
}

void EbeamerAudioProcessor::processCC(const MidiCC &cc, int value) {
    
    const String paramTag = ccToParamMap[cc];
    Value val = parameters.getParameterAsValue(paramTag);
    auto range = parameters.getParameterRange(paramTag);
    const bool isButton = range.interval == 1 && range.start == 0 && range.end == 1;
    if (isButton) {
        if (value == 127) {
            val.setValue(!((bool) val.getValue()));
        }
    } else {
        val.setValue(range.convertFrom0to1(value / 127.));
    }
    
}

void EbeamerAudioProcessor::startCCLearning(const String &p) {
    paramCCToLearn = p;
}

void EbeamerAudioProcessor::stopCCLearning() {
    paramCCToLearn = "";
}

String EbeamerAudioProcessor::getCCLearning() const {
    return paramCCToLearn;
}

const std::map<String, MidiCC> &EbeamerAudioProcessor::getParamToCCMapping() {
    return paramToCcMap;
}

void EbeamerAudioProcessor::processMidi(MidiBuffer &midiMessages) {
    
    // Loop over Midi messages
    for (const MidiMessageMetadata metadata : midiMessages){
        MidiMessage midiMess = metadata.getMessage();
        
        if (midiMess.isController()) {
            
            MidiCC cc = {midiMess.getChannel(), midiMess.getControllerNumber()};
            if (ccToParamMap.count(cc) > 0) {
                /** Process the CC message if mapped */
                processCC(cc, midiMess.getControllerValue());
            } else if (paramCCToLearn.length() > 0) {
                /** Remove then add the CC parameter */
                removeCCParamMapping(paramCCToLearn);
                insertCCParamMapping(cc, paramCCToLearn);
            }
        }
    }
    
    /** Clear all messages */
    midiMessages.clear();
    
}

void EbeamerAudioProcessor::processBlock(AudioBuffer<float> &buffer, MidiBuffer &midiMessages) {
    
    const auto startTick = Time::getHighResolutionTicks();
    
    GenericScopedLock<SpinLock> lock(processingLock);
    
    processMidi(midiMessages);
    
    /** If resources are not allocated this is an out-of-order request */
    if (!resourcesAllocated) {
        jassertfalse;
        return;
    }
    
    
    ScopedNoDenormals noDenormals;
    
    /**Apply input gain directly on input buffer  */
    micGain.setGainDecibels(*micGainParam);
    {
        auto block = juce::dsp::AudioBlock<float>(buffer).getSubsetChannelBlock(0, numActiveInputChannels);
        auto context = juce::dsp::ProcessContextReplacing<float>(block);
        micGain.process(context);
    }
    
    // Mic meter
    inputMeterDecay->push(buffer);
    
    /** Renew IIR coefficient if cut frequency changed */
    if (prevHpfFreq != (bool) *hpfFreqParam) {
        iirCoeffHPF = IIRCoefficients::makeHighPass(sampleRate, *hpfFreqParam);
        prevHpfFreq = *hpfFreqParam;
        for (auto &iirHPFfilter : iirHPFfilters) {
            iirHPFfilter.setCoefficients(iirCoeffHPF);
        }
    }
    
    /**Apply HPF directly on input buffer  */
    for (auto inChannel = 0; inChannel < numActiveInputChannels; ++inChannel) {
        iirHPFfilters[inChannel].processSamples(buffer.getWritePointer(inChannel), buffer.getNumSamples());
    }
    
    /** Set beams parameters */
    for (auto beamIdx = 0; beamIdx < 2; beamIdx++) {
        float beamDoaX = *steerBeamXParam[beamIdx];
        float beamDoaY = -(*steerBeamYParam[beamIdx]); //GUI and Beamforming use opposite vertical conventions
        beamDoaX = *frontFacingParam ? -beamDoaX : beamDoaX;
        BeamParameters beamParams = {beamDoaX,beamDoaY, *widthBeamParam[beamIdx]};
        beamformer->setBeamParameters(beamIdx, beamParams);
    }
    
    /** Call the beamformer  */
    beamformer->processBlock(buffer);
    
    /** Retrieve beamformer outputs */
    beamformer->getBeams(beamBuffer);
    
    /** Apply beams mute and volume */
    for (auto beamIdx = 0; beamIdx < 2; ++beamIdx) {
        if ((bool) *muteBeamParam[beamIdx] == false) {
            beamGain[beamIdx].setGainDecibels(*levelBeamParam[beamIdx]);
        } else {
            beamGain[beamIdx].setGainLinear(0);
        }
        auto block = dsp::AudioBlock<float>(beamBuffer).getSubsetChannelBlock(beamIdx, 1).getSubBlock(0,
                                                                                                      buffer.getNumSamples());
        auto contextToUse = dsp::ProcessContextReplacing<float>(block);
        beamGain[beamIdx].process(contextToUse);
    }
    
    /** Measure beam output volume */
    beamMeterDecay->push(beamBuffer);
    
    /** Clear buffer */
    buffer.clear();
    
    /** Sum beams in output channels */
    for (int outChannel = 0; outChannel < numActiveOutputChannels; ++outChannel) {
        /** Sum the contributes from each beam */
        for (int beamIdx = 0; beamIdx < 2; ++beamIdx) {
            auto channelBeamGain = panToLinearGain((float) *panBeamParam[beamIdx], outChannel == 0);
            buffer.addFrom(outChannel, 0, beamBuffer, beamIdx, 0, buffer.getNumSamples(), channelBeamGain);
        }
    }
    
    /** Update load */
    {
        const float elapsedTime = Time::highResolutionTicksToSeconds(Time::getHighResolutionTicks() - startTick);
        const float curLoad = elapsedTime / (maximumExpectedSamplesPerBlock / sampleRate);
        GenericScopedLock<SpinLock> lock(loadLock);
        load = (load * (1 - loadAlpha)) + (curLoad * loadAlpha);
    }
    
}

//==============================================================================
void EbeamerAudioProcessor::getStateInformation(MemoryBlock &destData) {
    /** Root XML */
    std::unique_ptr<XmlElement> xml(new XmlElement("eBeamerRoot"));
    
    /** Parameters state */
    auto state = parameters.copyState();
    XmlElement *xmlParams = new XmlElement(*state.createXml());
    xml->addChildElement(xmlParams);
    
    /** Save Midi CC - Params Maping */
    auto xmlMidi = xml->createNewChildElement("eBeamerMidiMap");
    for (auto m : paramToCcMap) {
        auto el = xmlMidi->createNewChildElement(m.first);
        el->setAttribute("channel", m.second.channel);
        el->setAttribute("number", m.second.number);
    }
    copyXmlToBinary(*xml, destData);
}

void EbeamerAudioProcessor::setStateInformation(const void *data, int sizeInBytes) {
    
    std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    
    if (xmlState.get() != nullptr) {
        if (xmlState->hasTagName("eBeamerRoot")) {
            for (auto rootElement : xmlState->getChildIterator()){
                if (rootElement->hasTagName(parameters.state.getType())) {
                    /** Parameters state */
                    parameters.replaceState(ValueTree::fromXml(*rootElement));
                    syncParametersToValueTree();
                } else if (rootElement->hasTagName("eBeamerMidiMap")) {
                    /** Load Midi CC - Params Maping */
                    ccToParamMap.clear();
                    paramToCcMap.clear();
                    stopCCLearning();
                    for (auto e : rootElement->getChildIterator()){
                        String tag = e->getTagName();
                        int channel = e->getIntAttribute("channel");
                        int number = e->getIntAttribute("number");
                        insertCCParamMapping({channel, number}, tag);
                    }
                }
            }
        }
    }
}

//==============================================================================
// Unchanged JUCE default functions
EbeamerAudioProcessor::~EbeamerAudioProcessor() {
}

const String EbeamerAudioProcessor::getName() const {
    return JucePlugin_Name;
}

bool EbeamerAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool EbeamerAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool EbeamerAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double EbeamerAudioProcessor::getTailLengthSeconds() const {
    return 0.0;
}

int EbeamerAudioProcessor::getNumPrograms() {
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int EbeamerAudioProcessor::getCurrentProgram() {
    return 0;
}

void EbeamerAudioProcessor::setCurrentProgram(int index) {
}

const String EbeamerAudioProcessor::getProgramName(int index) {
    return {};
}

void EbeamerAudioProcessor::changeProgramName(int index, const String &newName) {
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
    return new EbeamerAudioProcessor();
}

//==============================================================================
AudioProcessorEditor *EbeamerAudioProcessor::createEditor() {
    return new EBeamerAudioProcessorEditor(*this, valueTree);
}

bool EbeamerAudioProcessor::hasEditor() const {
    return true;
}

void EbeamerAudioProcessor::showConnectionErrorMessage (const String& messageText){
    juce::AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                            "OSC connection error",
                                            messageText,
                                            "OK");
}

//==============================================================================


/** Set a state parameter */
void EbeamerAudioProcessor::setParam(const Identifier& name, float newVal){
    auto param = parameters.getParameter(name);
    auto newVal01 = param->convertTo0to1(newVal);
    param->setValueNotifyingHost(newVal01);
}

/** Set a state parameter */
void EbeamerAudioProcessor::setParam(const Identifier& name, bool newVal){
    auto param = parameters.getParameter(name);
    param->setValueNotifyingHost(newVal);
}

/** Set a state parameter */
void EbeamerAudioProcessor::setParam(const Identifier& name, MicConfig newVal){
    auto param = parameters.getParameter(name);
    auto newVal01 = param->convertTo0to1(newVal);
    param->setValueNotifyingHost(newVal01);
}

/**
 Sync parameters changes to valueTree
 */
void EbeamerAudioProcessor::parameterChanged(const String &parameterID, float newValue) {
    if (parameterID == configIdentifier.toString()) {
        valueTree.setPropertyExcludingListener(this,configIdentifier, (int)newValue, nullptr);
        prepareToPlay(sampleRate, maximumExpectedSamplesPerBlock);
        return;
    }
    if (parameterID == frontIdentifier.toString()){
        valueTree.setPropertyExcludingListener(this,frontIdentifier, (bool)newValue, nullptr);
        return;
    }
    if (parameterID == gainIdentifier.toString()){
        valueTree.setPropertyExcludingListener(this,gainIdentifier, (float)newValue, nullptr);
        return;
    }
    if (parameterID == hpfIdentifier.toString()){
        valueTree.setPropertyExcludingListener(this,hpfIdentifier, (float)newValue, nullptr);
        return;
    }
    String identifier;
    for (auto beamIdx = 0; beamIdx < 2; beamIdx++) {
        identifier = (steerXIdentifierPrefix + String(beamIdx + 1));
        if (parameterID==identifier){
            valueTree.setPropertyExcludingListener(this,identifier, (float)newValue, nullptr);
            return;
        }
        identifier = (steerYIdentifierPrefix + String(beamIdx + 1));
        if (parameterID==identifier){
            valueTree.setPropertyExcludingListener(this,identifier, (float)newValue, nullptr);
            return;
        }
        identifier = (widthIdentifierPrefix + String(beamIdx + 1));
        if (parameterID==identifier){
            valueTree.setPropertyExcludingListener(this,identifier, (float)newValue, nullptr);
            return;
        }
        identifier = (panIdentifierPrefix + String(beamIdx + 1));
        if (parameterID==identifier){
            valueTree.setPropertyExcludingListener(this,identifier, (float)newValue, nullptr);
            return;
        }
        identifier = (levelIdentifierPrefix + String(beamIdx + 1));
        if (parameterID==identifier){
            valueTree.setPropertyExcludingListener(this,identifier, (float)newValue, nullptr);
            return;
        }
        identifier = (muteIdentifierPrefix + String(beamIdx + 1));
        if (parameterID==identifier){
            valueTree.setPropertyExcludingListener(this,identifier, (bool)newValue, nullptr);
            return;
        }
    }
}

/**
 Sync valueTree changes to parameters
 */
void EbeamerAudioProcessor::valueTreePropertyChanged (ValueTree &vt, const Identifier &property){
    if (property==configIdentifier){
        setParam(configIdentifier,static_cast<MicConfig>(int(vt[property])));
        return;
    }
    if (property==frontIdentifier){
        setParam(frontIdentifier,bool(vt[property]));
        return;
    }
    if (property==gainIdentifier){
        setParam(gainIdentifier,float(vt[property]));
        return;
    }
    if (property==hpfIdentifier){
        setParam(hpfIdentifier,float(vt[property]));
        return;
    }
    Identifier identifier;
    for (auto beamIdx = 0; beamIdx < 2; beamIdx++) {
        identifier = (steerXIdentifierPrefix + String(beamIdx + 1));
        if (property==identifier){
            setParam(identifier,float(vt[property]));
            return;
        }
        identifier = (steerYIdentifierPrefix + String(beamIdx + 1));
        if (property==identifier){
            setParam(identifier,float(vt[property]));
            return;
        }
        identifier = (widthIdentifierPrefix + String(beamIdx + 1));
        if (property==identifier){
            setParam(identifier,float(vt[property]));
            return;
        }
        identifier = (panIdentifierPrefix + String(beamIdx + 1));
        if (property==identifier){
            setParam(identifier,float(vt[property]));
            return;
        }
        identifier = (levelIdentifierPrefix + String(beamIdx + 1));
        if (property==identifier){
            setParam(identifier,float(vt[property]));
            return;
        }
        identifier = (muteIdentifierPrefix + String(beamIdx + 1));
        if (property==identifier){
            setParam(identifier,bool(vt[property]));
            return;
        }
    }
}

void EbeamerAudioProcessor::syncParametersToValueTree(){
    valueTree.setPropertyExcludingListener(this,configIdentifier, (int)*configParam, nullptr);
    valueTree.setPropertyExcludingListener(this,frontIdentifier, (bool)*frontFacingParam, nullptr);
    valueTree.setPropertyExcludingListener(this,gainIdentifier, (float)*micGainParam, nullptr);
    valueTree.setPropertyExcludingListener(this,hpfIdentifier, (float)*hpfFreqParam, nullptr);
    
    Identifier identifier;
    for (auto beamIdx = 0; beamIdx < 2; beamIdx++) {
        identifier = (steerXIdentifierPrefix + String(beamIdx + 1));
        valueTree.setPropertyExcludingListener(this,identifier, (float)(*steerBeamXParam[beamIdx]), nullptr);
        identifier = (steerYIdentifierPrefix + String(beamIdx + 1));
        valueTree.setPropertyExcludingListener(this,identifier, (float)(*steerBeamYParam[beamIdx]), nullptr);
        identifier = (widthIdentifierPrefix + String(beamIdx + 1));
        valueTree.setPropertyExcludingListener(this,identifier, (float)(*widthBeamParam[beamIdx]), nullptr);
        identifier = (panIdentifierPrefix + String(beamIdx + 1));
        valueTree.setPropertyExcludingListener(this,identifier, (float)(*panBeamParam[beamIdx]), nullptr);
        identifier = (levelIdentifierPrefix + String(beamIdx + 1));
        valueTree.setPropertyExcludingListener(this,identifier, (float)(*levelBeamParam[beamIdx]), nullptr);
        identifier = (muteIdentifierPrefix + String(beamIdx + 1));
        valueTree.setPropertyExcludingListener(this,identifier, (bool)(*muteBeamParam[beamIdx]), nullptr);
    }
}

/**
 Push cpu load, meters, energy updates at a human rate
 */
void EbeamerAudioProcessor::timerCallback(){
    
    valueTree.setProperty(cpuIdentifier, load, nullptr);
    valueTree.setProperty(outMeter1Identifier, beamMeterDecay->get(0), nullptr);
    valueTree.setProperty(outMeter2Identifier, beamMeterDecay->get(1), nullptr);
    valueTree.setProperty(inMetersIdentifier, inputMeterDecay->get(), nullptr);
    valueTree.setProperty(energyIdentifier, beamformer->getDoaEnergy(), nullptr);
    
}
