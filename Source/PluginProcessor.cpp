#include "PluginProcessor.h"
#include "PluginEditor.h"

/* Allocate static members to use them in the constructor */
const int EbeamerAudioProcessor::numBeams;
const int EbeamerAudioProcessor::numDoas;

//==============================================================================
// Helper functions
AudioProcessorValueTreeState::ParameterLayout initializeParameters() {
    
    std::vector<std::unique_ptr<RangedAudioParameter>> params;
    
    // Values in dB
    params.push_back(std::make_unique<AudioParameterChoice>("config", //tag
                                                        "Configuration", //name
                                                        micConfigLabels, //choices
                                                        0 //default
                                                        ));
    
    params.push_back(std::make_unique<AudioParameterBool>("frontFacing", //tag
                                                           "Front facing", //name
                                                           false //default
                                                           ));
    
    params.push_back(std::make_unique<AudioParameterFloat>("gainMic", //tag
                                                        "Mic gain", //name
                                                        0.0f, //min
                                                        40.0f, //max
                                                        20.0f //default
                                                        ));
    
    // Values in Hz
    params.push_back(std::make_unique<AudioParameterFloat>("hpf", //tag
                                                        "HPF",
                                                        20.0f, //min
                                                        500.0f, //max
                                                        250.0f //default
                                                        ));
    
    {
        std::ostringstream ssTag;
        std::ostringstream ssName;
        for (auto beamIdx = 0; beamIdx < EbeamerAudioProcessor::numBeams; ++beamIdx){
            ssTag.str(std::string());
            ssName.str(std::string());
            ssTag << "steerBeam" << (beamIdx+1);
            ssName << "Steering beam " << (beamIdx+1);
            auto defaultDirection = beamIdx == 0 ?  -0.5 : 0.5;
            params.push_back(std::make_unique<AudioParameterFloat>(ssTag.str(), //tag
                                                                              ssName.str(), //name
                                                                              -1.0f, //min
                                                                              1.0f, //max
                                                                              defaultDirection //default
                                                                              ));
            ssTag.str(std::string());
            ssName.str(std::string());
            ssTag << "widthBeam" << (beamIdx+1);
            ssName << "Width beam " << (beamIdx+1);
            params.push_back(std::make_unique<AudioParameterFloat>(ssTag.str(), //tag
                                                                           ssName.str(), //name
                                                                           0.0f, //min
                                                                           1.0f,//max
                                                                           0.3f//default
                                                                           ));
            ssTag.str(std::string());
            ssName.str(std::string());
            ssTag << "panBeam" << (beamIdx+1);
            ssName << "Pan beam " << (beamIdx+1);
            auto defaultPan = beamIdx == 0 ?  -0.5 : 0.5;
            params.push_back(std::make_unique<AudioParameterFloat>(ssTag.str(), //tag
                                                                         ssName.str(), //name
                                                                         -1.0f, //min
                                                                         1.0f, //max
                                                                         defaultPan //default
                                                                         ));
            ssTag.str(std::string());
            ssName.str(std::string());
            ssTag << "levelBeam" << (beamIdx+1);
            ssName << "Level beam " << (beamIdx+1);
            params.push_back(std::make_unique<AudioParameterFloat>(ssTag.str(), //tag
                                                                           ssName.str(),
                                                                           -10.0f, //min
                                                                           10.0f, //max
                                                                           0.0f //default
                                                                           ));
            
            ssTag.str(std::string());
            ssName.str(std::string());
            ssTag << "muteBeam" << (beamIdx+1);
            ssName << "Mute beam " << (beamIdx+1);
            params.push_back(std::make_unique<AudioParameterBool>(ssTag.str(), //tag
                                                                         ssName.str(), //name
                                                                         false //default
                                                                         ));
            
        }
    }
    
    return { params.begin(), params.end() };
}


//==============================================================================
EbeamerAudioProcessor::EbeamerAudioProcessor()
: AudioProcessor (BusesProperties() //The default bus layout accommodates for 4 buses of 16 channels each.
                  .withInput  ("eStick#1",  AudioChannelSet::ambisonic(3), true)
                  .withInput  ("eStick#2",  AudioChannelSet::ambisonic(3), true)
                  .withInput  ("eStick#3",  AudioChannelSet::ambisonic(3), true)
                  .withInput  ("eStick#4",  AudioChannelSet::ambisonic(3), true)
                  .withOutput ("Output", AudioChannelSet::stereo(), true)
                  ),parameters(*this,&undo,Identifier("eBeamer"),initializeParameters())
{
    
    /** Get parameters pointers */
    configParam = parameters.getRawParameterValue("config");
    parameters.addParameterListener("config", this);
    frontFacingParam = parameters.getRawParameterValue("frontFacing");
    hpfFreqParam = parameters.getRawParameterValue("hpf");
    micGainParam = parameters.getRawParameterValue("gainMic");
    
    std::ostringstream ssTag;
    std::ostringstream ssName;
    for (auto beamIdx=0;beamIdx<numBeams;beamIdx++){
        ssTag.str(std::string());
        ssName.str(std::string());
        ssTag << "steerBeam" << (beamIdx+1);
        steeringBeamParam[beamIdx] = parameters.getRawParameterValue(ssTag.str());
        
        ssTag.str(std::string());
        ssName.str(std::string());
        ssTag << "widthBeam" << (beamIdx+1);
        widthBeamParam[beamIdx] = parameters.getRawParameterValue(ssTag.str());
        
        ssTag.str(std::string());
        ssName.str(std::string());
        ssTag << "panBeam" << (beamIdx+1);
        panBeamParam[beamIdx] = parameters.getRawParameterValue(ssTag.str());
        
        ssTag.str(std::string());
        ssName.str(std::string());
        ssTag << "levelBeam" << (beamIdx+1);
        levelBeamParam[beamIdx] = parameters.getRawParameterValue(ssTag.str());
        
        ssTag.str(std::string());
        ssName.str(std::string());
        ssTag << "muteBeam" << (beamIdx+1);
        muteBeamParam[beamIdx] = parameters.getRawParameterValue(ssTag.str());
    }
    

    /** Initialize the beamformer */
    beamformer = std::make_unique<Beamformer>(numBeams,numDoas);
    beamformer->setMicConfig(static_cast<MicConfig>((int)*configParam));
    
}

//==============================================================================
bool EbeamerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const{
    // This plug-in supports up to 4 eSticks, for a total amount of 64 channels in input.
    // VST3 allows for a maximum of 25 channels per bus.
    // To make things simpler in terms of patching, each input bus counts for at most 16 channels.
    // This configuration allows REAPER to be configured with a 64 channels track.
    for (auto bus : layouts.inputBuses){
        if ( bus.size() > 16 ){
            return false;
        }
    }
    for (auto bus : layouts.outputBuses){
        if ( bus.size() > 16 ){
            // We have to allow the output bus to grow to the size of the input bus for compatibility with REAPER
            return false;
        }
    }
    if ((layouts.getMainInputChannels() < 2) or (layouts.getMainOutputChannels() < 2)){
        // In any case don't allow less than 2 input and 2 output channels
        return false;
    }
    return true;
}

//==============================================================================
void EbeamerAudioProcessor::prepareToPlay (double sampleRate_, int maximumExpectedSamplesPerBlock_)
{
    
    GenericScopedLock<SpinLock> lock(processingLock);
    
    sampleRate = sampleRate_;
    maximumExpectedSamplesPerBlock = maximumExpectedSamplesPerBlock_;
    
    /** Number of active input channels */
    numActiveInputChannels = getTotalNumInputChannels();
    
    /** Number of active output channels */
    numActiveOutputChannels = jmin(numBeams,getTotalNumOutputChannels());
    
    /** Initialize the input gain */
    micGain.reset();
    micGain.prepare({sampleRate, static_cast<uint32>(maximumExpectedSamplesPerBlock),numActiveInputChannels});
    micGain.setGainDecibels(*micGainParam);
    micGain.setRampDurationSeconds(gainTimeConst);
    
    /** Initialize the High Pass Filters */
    iirHPFfilters.clear();
    iirHPFfilters.resize(numActiveInputChannels);
    prevHpfFreq = 0;
    
    /** Initialize the beamformer */
    beamformer->prepareToPlay(sampleRate, maximumExpectedSamplesPerBlock);
    
    /** Initialize beams' buffer  */
    beamBuffer.setSize(numBeams, maximumExpectedSamplesPerBlock);
    
    /** Initialize beam level gains */
    for (auto beamIdx = 0; beamIdx < numBeams; ++beamIdx){
        beamGain[beamIdx].reset();
        beamGain[beamIdx].prepare({sampleRate, static_cast<uint32>(maximumExpectedSamplesPerBlock),1});
        beamGain[beamIdx].setGainDecibels(*levelBeamParam[beamIdx]);
        beamGain[beamIdx].setRampDurationSeconds(gainTimeConst);
    }
    
    /** initialize meters */
    inputMeterDecay = std::make_unique<MeterDecay>(sampleRate,metersDecay,maximumExpectedSamplesPerBlock,numActiveInputChannels);
    inputMeters.resize(numActiveInputChannels);
    beamMeterDecay = std::make_unique<MeterDecay>(sampleRate,metersDecay,maximumExpectedSamplesPerBlock,numBeams);
    beamMeters.resize(numBeams);
    
    resourcesAllocated = true;
    
}

void EbeamerAudioProcessor::releaseResources()
{
 
    GenericScopedLock<SpinLock> lock(processingLock);
    
    resourcesAllocated = false;

    /** Clear beam buffer */
    beamBuffer.setSize(numBeams, 0);
    
    /** Clear the HPF */
    iirHPFfilters.clear();
    
    /** Clear the Beamformer */
    beamformer->releaseResources();
}

void EbeamerAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    
    const auto startTick = Time::getHighResolutionTicks();
    
    GenericScopedLock<SpinLock> lock(processingLock);
    
    /** If resources are not allocated this is an out-of-order request */
    if (!resourcesAllocated){
        jassertfalse;
        return;
    }
    
    ScopedNoDenormals noDenormals;
    
    /**Apply input gain directly on input buffer  */
    micGain.setGainDecibels(*micGainParam);
    {
        auto block = juce::dsp::AudioBlock<float>(buffer).getSubsetChannelBlock(0, numActiveInputChannels);
        auto context = juce::dsp::ProcessContextReplacing<float> (block);
        micGain.process(context);
    }
    
    // Mic meter
    inputMeterDecay->push(buffer);
    {
        GenericScopedLock<SpinLock> lock(inputMetersLock);
        inputMeters = inputMeterDecay->get();
    }
    
    /** Renew IIR coefficient if cut frequency changed */
    if(prevHpfFreq != (bool)*hpfFreqParam){
        iirCoeffHPF = IIRCoefficients::makeHighPass(sampleRate, *hpfFreqParam);
        prevHpfFreq = *hpfFreqParam;
        for (auto& iirHPFfilter : iirHPFfilters){
            iirHPFfilter.setCoefficients(iirCoeffHPF);
        }
    }

    /**Apply HPF directly on input buffer  */
    for (auto inChannel = 0; inChannel < numActiveInputChannels; ++inChannel){
        iirHPFfilters[inChannel].processSamples(buffer.getWritePointer(inChannel), buffer.getNumSamples());
    }

    /** Set beams parameters */
    for (auto beamIdx = 0;beamIdx< numBeams; beamIdx++){
        float beamDoa = *steeringBeamParam[beamIdx];
        beamDoa = *frontFacingParam ? -beamDoa : beamDoa;
        BeamParameters beamParams = {beamDoa,*widthBeamParam[beamIdx]};
        beamformer->setBeamParameters(beamIdx, beamParams);
    }

    /** Call the beamformer  */
    beamformer->processBlock(buffer);

    /** Retrieve beamformer outputs */
    beamformer->getBeams(beamBuffer);

    /** Apply beams mute and volume */
    for (auto beamIdx = 0; beamIdx < numBeams; ++beamIdx){
        if ((bool)*muteBeamParam[beamIdx] == false){
            beamGain[beamIdx].setGainDecibels(*levelBeamParam[beamIdx]);
        }else{
            beamGain[beamIdx].setGainLinear(0);
        }
        auto block = dsp::AudioBlock<float>(beamBuffer).getSubsetChannelBlock(beamIdx, 1).getSubBlock(0, buffer.getNumSamples());
        auto contextToUse = dsp::ProcessContextReplacing<float> (block);
        beamGain[beamIdx].process(contextToUse);
    }

    /** Measure beam output volume */
    beamMeterDecay->push(beamBuffer);
    {
        GenericScopedLock<SpinLock> lock(beamMetersLock);
        beamMeters = beamMeterDecay->get();
    }

    /** Clear buffer */
    buffer.clear();

    /** Sum beams in output channels */
    for (int outChannel = 0; outChannel < numActiveOutputChannels; ++outChannel){
        /** Sum the contributes from each beam */
        for (int beamIdx = 0; beamIdx < numBeams; ++beamIdx){
            auto channelBeamGain = panToLinearGain((float)*panBeamParam[beamIdx],outChannel==0);
            buffer.addFrom(outChannel, 0, beamBuffer, beamIdx, 0, buffer.getNumSamples(), channelBeamGain);
        }
    }
    
    /** Update load */
    {
        const float elapsedTime = Time::highResolutionTicksToSeconds(Time::getHighResolutionTicks() - startTick);
        const float curLoad = elapsedTime / (maximumExpectedSamplesPerBlock/sampleRate);
        GenericScopedLock<SpinLock> lock(loadLock);
        load = (load*(1-loadAlpha))+(curLoad*loadAlpha);
    }
    
}

const AudioProcessorValueTreeState& EbeamerAudioProcessor::getParams() const{
    return parameters;
}


void EbeamerAudioProcessor::parameterChanged (const String &parameterID, float newValue){
    if (parameterID == "config"){
        setMicConfig(static_cast<MicConfig>((int)(newValue)));
    }
}

//==============================================================================
void EbeamerAudioProcessor::setMicConfig(const MicConfig& mc){
    beamformer->setMicConfig(mc);
    prepareToPlay(sampleRate, maximumExpectedSamplesPerBlock);
}

//==============================================================================
float EbeamerAudioProcessor::getAverageLoad() const{
    GenericScopedLock<SpinLock> lock(loadLock);
    return load;
}

//==============================================================================
void EbeamerAudioProcessor::getStateInformation (MemoryBlock& destData){
    auto state = parameters.copyState();
    std::unique_ptr<XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void EbeamerAudioProcessor::setStateInformation (const void* data, int sizeInBytes){

    std::unique_ptr<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (ValueTree::fromXml (*xmlState));
}

//==============================================================================
// Meters

float EbeamerAudioProcessor::getBeamMeter(int channel){
    GenericScopedLock<SpinLock> lock(beamMetersLock);
    return beamMeters[channel];
}

std::vector<float> EbeamerAudioProcessor::getInputMeters(){
    GenericScopedLock<SpinLock> lock(inputMetersLock);
    return inputMeters;
}

//==============================================================================
// Beamformer
std::unique_ptr<Beamformer>& EbeamerAudioProcessor::getBeamformer(){
    return beamformer;
}
const std::unique_ptr<Beamformer>& EbeamerAudioProcessor::getBeamformer() const{
    return beamformer;
}

//==============================================================================
// Unchanged JUCE default functions
EbeamerAudioProcessor::~EbeamerAudioProcessor()
{
}

const String EbeamerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool EbeamerAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool EbeamerAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool EbeamerAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double EbeamerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int EbeamerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int EbeamerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void EbeamerAudioProcessor::setCurrentProgram (int index)
{
}

const String EbeamerAudioProcessor::getProgramName (int index)
{
    return {};
}

void EbeamerAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EbeamerAudioProcessor();
}

//==============================================================================
AudioProcessorEditor* EbeamerAudioProcessor::createEditor()
{
    return new JucebeamAudioProcessorEditor (*this,parameters);
}
bool EbeamerAudioProcessor::hasEditor() const
{
    return true;
}
