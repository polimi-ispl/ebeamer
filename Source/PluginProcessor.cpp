#include "PluginProcessor.h"
#include "PluginEditor.h"

/* Allocate static members to use them in the constructor */
const int EbeamerAudioProcessor::numBeams;
const int EbeamerAudioProcessor::numDoas;

//==============================================================================
EbeamerAudioProcessor::EbeamerAudioProcessor()
: AudioProcessor (BusesProperties() //The default bus layout accommodates for 4 buses of 16 channels each.
                  .withInput  ("eStick#1",  AudioChannelSet::ambisonic(3), true)
                  .withInput  ("eStick#2",  AudioChannelSet::ambisonic(3), true)
                  .withInput  ("eStick#3",  AudioChannelSet::ambisonic(3), true)
                  .withInput  ("eStick#4",  AudioChannelSet::ambisonic(3), true)
                  .withOutput ("Output", AudioChannelSet::stereo(), true)
                  )
{
    /** Initialize here everything that doesn't depend on sample rate, buffer size, bus configuration */
    initializeParameters();
    
    /** Initialize the beamformer */
    beamformer = std::make_unique<Beamformer>(*this,numBeams,numDoas);

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
void EbeamerAudioProcessor::prepareToPlay (double sampleRate, int maximumExpectedSamplesPerBlock)
{
    
    /** Number of active input channels */
    numActiveInputChannels = getTotalNumInputChannels();
    
    /** Number of active output channels */
    numActiveOutputChannels = jmin(numBeams,getTotalNumOutputChannels());
    
    /** Initialize the input gain */
    micGain.reset();
    micGain.prepare({sampleRate, static_cast<uint32>(maximumExpectedSamplesPerBlock),numActiveInputChannels});
    micGain.setGainDecibels(micGainParam->get());
    micGain.setRampDurationSeconds(gainTimeConst);
    
    /** Initialize the High Pass Filters */
    iirHPFfilters.clear();
    iirCoeffHPF = IIRCoefficients::makeHighPass(getSampleRate(), hpfFreqParam->get());
    
    iirHPFfilters.resize(numActiveInputChannels);
    for (auto& iirHPFfilter : iirHPFfilters){
        iirHPFfilter = std::make_unique<IIRFilter>();
        iirHPFfilter->setCoefficients(iirCoeffHPF);
    }
    
    /** Initialize the beamformer */
    beamformer->prepareToPlay(sampleRate, maximumExpectedSamplesPerBlock, numActiveInputChannels);
    
    /** Initialize beams' buffer  */
    beamBuffer.setSize(numBeams, maximumExpectedSamplesPerBlock);
    
    /** Initialize beam level gains */
    for (auto beamIdx = 0; beamIdx < numBeams; ++beamIdx){
        beamGain[beamIdx].reset();
        beamGain[beamIdx].prepare({sampleRate, static_cast<uint32>(maximumExpectedSamplesPerBlock),1});
        beamGain[beamIdx].setGainDecibels(levelBeamParam[beamIdx]->get());
        beamGain[beamIdx].setRampDurationSeconds(gainTimeConst);
    }
    
    //    // Meters
    //    inputMeterDecay = std::make_unique<MeterDecay>(sampleRate,METERS_DECAY,samplesPerBlock,numInputChannels);
    //    inputMeters.resize(numInputChannels);
    //    beamMeterDecay = std::make_unique<MeterDecay>(sampleRate,METERS_DECAY,samplesPerBlock,NUM_BEAMS);
    //    beamMeters.resize(NUM_BEAMS);
    
}

void EbeamerAudioProcessor::releaseResources()
{
    /** Clear beam buffer */
    beamBuffer.setSize(numBeams, 0);
    
    /** Clear the HPF */
    iirHPFfilters.clear();
    
    /** Clear the Beamformer */
    beamformer->releaseResources();
}

void EbeamerAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    
    /**Apply input gain directly on input buffer  */
    micGain.setGainDecibels(micGainParam->get());
    {
        auto block = juce::dsp::AudioBlock<float>(buffer).getSubsetChannelBlock(0, numActiveInputChannels);
        auto context = juce::dsp::ProcessContextReplacing<float> (block);
        micGain.process(context);
    }
    
    /** Renew IIR coefficient if cut frequency changed */
    if(prevHpfFreq != hpfFreqParam->get()){
        iirCoeffHPF = IIRCoefficients::makeHighPass(getSampleRate(), hpfFreqParam->get());
        prevHpfFreq = hpfFreqParam->get();
        for (auto& iirHPFfilter : iirHPFfilters){
            iirHPFfilter->setCoefficients(iirCoeffHPF);
        }
    }
    
    /**Apply HPF directly on input buffer  */
    for (auto inChannel = 0; inChannel < numActiveInputChannels; ++inChannel){
        iirHPFfilters[inChannel]->processSamples(buffer.getWritePointer(inChannel), buffer.getNumSamples());
    }
    
    /** Set beams parameters */
    for (auto beamIdx = 0;beamIdx< numBeams; beamIdx++){
        BeamParameters beamParams = {steeringBeamParam[beamIdx]->get(),widthBeamParam[beamIdx]->get()};
        beamformer->setBeamParameters(beamIdx, beamParams);
    }
    
    /** Call the beamformer  */
    beamformer->processBlock(buffer);
    
    /** Retrieve beamformer outputs */
    beamformer->getBeams(beamBuffer);
    
    /** Apply beams gain */
    for (auto beamIdx = 0; beamIdx < numBeams; ++beamIdx){
        beamGain[beamIdx].setGainDecibels(levelBeamParam[beamIdx]->get());
        auto block = dsp::AudioBlock<float>(beamBuffer).getSubsetChannelBlock(beamIdx, 1).getSubBlock(0, buffer.getNumSamples());
        auto contextToUse = dsp::ProcessContextReplacing<float> (block);
        beamGain[beamIdx].process(contextToUse);
    }
    
    /** Clear buffer */
    buffer.clear();
    
    /** Sum beams in output channels */
    for (int outChannel = 0; outChannel < numActiveOutputChannels; ++outChannel){
        /** Sum the contributes from each beam */
        for (int beamIdx = 0; beamIdx < numBeams; ++beamIdx){
            if (muteBeamParam[beamIdx]->get() == false){
                auto channelBeamGain = panToLinearGain(panBeamParam[beamIdx],outChannel==0);
                buffer.addFrom(outChannel, 0, beamBuffer, beamIdx, 0, buffer.getNumSamples(), channelBeamGain);
            }
        }
    }
    
    //    // Mic meter
    //    inputMeterDecay->push(buffer);
    //    {
    //        GenericScopedLock<SpinLock> lock(inputMetersLock);
    //        inputMeters = inputMeterDecay->get();
    //    }
    //
    //    // Compute and store fft for all input channels, so that DOA thread can operate
    //    {
    //        GenericScopedLock<SpinLock> lock(fftInputLock);
    //        inputsFFT.setTimeSeries(buffer);
    //        inputsFFT.prepareForConvolution();
    //        newFftInputDataAvailable = true;
    //    }
    //
    //    // Per input-channel processing
    //    mimoProcessor->newBlock(beamsBuffer);
    //    for (int inChannel = 0; inChannel < totalNumInputChannels; ++inChannel)
    //    {
    //        // Beam dependent processing
    //        for (int beamIdx = 0; beamIdx < NUM_BEAMS; ++beamIdx)
    //        {
    //            // Determine steering index
    //            int steeringIdx = roundToInt(((steeringBeam[beamIdx]->get() + 1)/2.)*(numSteeringDirections-1));
    //            // Determine beam width index
    //            int beamWidthIdx = roundToInt(widthBeam[beamIdx]->get()*(numBeamwidthChoices-1));
    //
    //            mimoProcessor->processBlock(inputsFFT,inChannel,beamsBuffer,beamIdx,steeringIdx,beamWidthIdx);
    //        }
    //    }
    //
    //
    //    // Beam meters
    //    beamMeterDecay->push(beamsBuffer);
    //    {
    //        GenericScopedLock<SpinLock> lock(beamMetersLock);
    //        beamMeters = beamMeterDecay->get();
    //    }
    //
    
}


//==============================================================================
AudioProcessorEditor* EbeamerAudioProcessor::createEditor()
{
    //    return new JucebeamAudioProcessorEditor (*this);
}
bool EbeamerAudioProcessor::hasEditor() const
{
    //    return true;
    return false;
}

//==============================================================================
void EbeamerAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    std::ostringstream stringStreamTag;
    std::unique_ptr<XmlElement> xml (new XmlElement ("eBeamer"));
    for (uint8 beamIdx = 0; beamIdx < numBeams; ++beamIdx){
        stringStreamTag.str(std::string());
        stringStreamTag << "steerBeam" << (beamIdx+1);
        xml->setAttribute(Identifier(stringStreamTag.str()), (double) *(steeringBeamParam[beamIdx]));
        
        stringStreamTag.str(std::string());
        stringStreamTag << "widthBeam" << (beamIdx+1);
        xml->setAttribute(Identifier(stringStreamTag.str()), (double) *(widthBeamParam[beamIdx]));
        
        stringStreamTag.str(std::string());
        stringStreamTag << "panBeam" << (beamIdx+1);
        xml->setAttribute(Identifier(stringStreamTag.str()), (double) *(panBeamParam[beamIdx]));
        
        stringStreamTag.str(std::string());
        stringStreamTag << "levelBeam" << (beamIdx+1);
        xml->setAttribute(Identifier(stringStreamTag.str()), (double) *(levelBeamParam[beamIdx]));
        
        stringStreamTag.str(std::string());
        stringStreamTag << "muteBeam" << (beamIdx+1);
        xml->setAttribute(Identifier(stringStreamTag.str()), (bool) *(muteBeamParam[beamIdx]));
        
    }
    stringStreamTag.str(std::string());
    stringStreamTag << "hpfFreq";
    xml->setAttribute(Identifier(stringStreamTag.str()), (double) *(hpfFreqParam));
    
    stringStreamTag.str(std::string());
    stringStreamTag << "gainMic";
    xml->setAttribute(Identifier(stringStreamTag.str()), (double) *(micGainParam));
    
    copyXmlToBinary (*xml, destData);
}

void EbeamerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::ostringstream stringStreamTag;
    std::unique_ptr<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr){
        if (xmlState->hasTagName ("eBeamer")){
            for (uint8 beamIdx = 0; beamIdx < numBeams; ++beamIdx){
                stringStreamTag.str(std::string());
                stringStreamTag << "steerBeam" << (beamIdx+1);
                *(steeringBeamParam[beamIdx]) = xmlState->getDoubleAttribute (Identifier(stringStreamTag.str()), 0.0);
                
                stringStreamTag.str(std::string());
                stringStreamTag << "widthBeam" << (beamIdx+1);
                *(widthBeamParam[beamIdx]) = xmlState->getDoubleAttribute (Identifier(stringStreamTag.str()), 0.0);
                
                stringStreamTag.str(std::string());
                stringStreamTag << "panBeam" << (beamIdx+1);
                *(panBeamParam[beamIdx]) = xmlState->getDoubleAttribute (Identifier(stringStreamTag.str()), 0.0);
                
                stringStreamTag.str(std::string());
                stringStreamTag << "levelBeam" << (beamIdx+1);
                *(levelBeamParam[beamIdx]) = xmlState->getDoubleAttribute (Identifier(stringStreamTag.str()), 0.0);
                
                stringStreamTag.str(std::string());
                stringStreamTag << "muteBeam" << (beamIdx+1);
                *(muteBeamParam[beamIdx]) = xmlState->getBoolAttribute(Identifier(stringStreamTag.str()), false);
                
            }
            stringStreamTag.str(std::string());
            stringStreamTag << "hpfFreq";
            *(hpfFreqParam) = xmlState->getDoubleAttribute (Identifier(stringStreamTag.str()), 250.0);
            
            stringStreamTag.str(std::string());
            stringStreamTag << "gainMic";
            *(micGainParam) = xmlState->getDoubleAttribute (Identifier(stringStreamTag.str()), 20.0);
        }
    }
}


//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EbeamerAudioProcessor();
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
// Helper functions
void EbeamerAudioProcessor::initializeParameters() {
    
    // Values in dB
    addParameter(micGainParam = new AudioParameterFloat("gainMic", //tag
                                                        "Mic gain", //name
                                                        0.0f, //min
                                                        40.0f, //max
                                                        20.0f //default
                                                        ));
    
    // Values in Hz
    addParameter(hpfFreqParam = new AudioParameterFloat("hpf", //tag
                                                        "HPF",
                                                        20.0f, //min
                                                        500.0f, //max
                                                        250.0f //default
                                                        ));
    
    {
        std::ostringstream ssTag;
        std::ostringstream ssName;
        for (auto beamIdx = 0; beamIdx < numBeams; ++beamIdx){
            ssTag.str(std::string());
            ssName.str(std::string());
            ssTag << "steerBeam" << (beamIdx+1);
            ssName << "Steering beam " << (beamIdx+1);
            auto defaultDirection = beamIdx == 0 ?  -0.5 : 0.5;
            addParameter(steeringBeamParam[beamIdx] = new AudioParameterFloat(ssTag.str(), //tag
                                                                              ssName.str(), //name
                                                                              -1.0f, //min
                                                                              1.0f, //max
                                                                              defaultDirection //default
                                                                              ));
            ssTag.str(std::string());
            ssName.str(std::string());
            ssTag << "widthBeam" << (beamIdx+1);
            ssName << "Width beam " << (beamIdx+1);
            addParameter(widthBeamParam[beamIdx] = new AudioParameterFloat(ssTag.str(), //tag
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
            addParameter(panBeamParam[beamIdx] = new AudioParameterFloat(ssTag.str(), //tag
                                                                         ssName.str(), //name
                                                                         -1.0f, //min
                                                                         1.0f, //max
                                                                         defaultPan //default
                                                                         ));
            ssTag.str(std::string());
            ssName.str(std::string());
            ssTag << "levelBeam" << (beamIdx+1);
            ssName << "Level beam " << (beamIdx+1);
            addParameter(levelBeamParam[beamIdx] = new AudioParameterFloat(ssTag.str(), //tag
                                                                           ssName.str(),
                                                                           -10.0f, //min
                                                                           10.0f, //max
                                                                           0.0f //default
                                                                           ));
            
            ssTag.str(std::string());
            ssName.str(std::string());
            ssTag << "muteBeam" << (beamIdx+1);
            ssName << "Mute beam " << (beamIdx+1);
            addParameter(muteBeamParam[beamIdx] = new AudioParameterBool(ssTag.str(), //tag
                                                                         ssName.str(), //name
                                                                         false //default
                                                                         ));
            
        }
    }
    
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
