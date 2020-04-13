#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
JucebeamAudioProcessor::JucebeamAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                       .withInput  ("Input",  AudioChannelSet::ambisonic(3), true)
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                       )
#endif
{
    
    
    // Initialize parameters
    std::ostringstream stringStreamTag;
    std::ostringstream stringStreamName;
    for (uint8 beamIdx = 0; beamIdx < NUM_BEAMS; ++beamIdx){
        stringStreamTag.str(std::string());
        stringStreamName.str(std::string());
        stringStreamTag << "steerBeam" << (beamIdx+1);
        stringStreamName << "Steering beam " << (beamIdx+1);
        auto defaultDirection = beamIdx == 0 ?  -0.5 : 0.5;
        addParameter(steeringBeam[beamIdx] = new AudioParameterFloat(stringStreamTag.str(),
                                                               stringStreamName.str(),
                                                               -1.0f,
                                                               1.0f,
                                                               defaultDirection));
        stringStreamTag.str(std::string());
        stringStreamName.str(std::string());
        stringStreamTag << "widthBeam" << (beamIdx+1);
        stringStreamName << "Width beam " << (beamIdx+1);
        addParameter(widthBeam[beamIdx] = new AudioParameterFloat(stringStreamTag.str(),
                                                                  stringStreamName.str(),
                                                            0.0f,
                                                            1.0f,
                                                            0.3f));
        stringStreamTag.str(std::string());
        stringStreamName.str(std::string());
        stringStreamTag << "panBeam" << (beamIdx+1);
        stringStreamName << "Pan beam " << (beamIdx+1);
        auto defaultPan = beamIdx == 0 ?  -0.5 : 0.5;
        addParameter(panBeam[beamIdx] = new AudioParameterFloat(stringStreamTag.str(),
                                                                stringStreamName.str(),
                                                          -1.0f,
                                                          1.0f,
                                                          defaultPan));
        stringStreamTag.str(std::string());
        stringStreamName.str(std::string());
        stringStreamTag << "levelBeam" << (beamIdx+1);
        stringStreamName << "Level beam " << (beamIdx+1);
        addParameter(levelBeam[beamIdx] = new AudioParameterFloat(stringStreamTag.str(),
                                                                 stringStreamName.str(),
                                                           -10.0f,
                                                           10.0f,
                                                           0.0f));
        
        stringStreamTag.str(std::string());
        stringStreamName.str(std::string());
        stringStreamTag << "muteBeam" << (beamIdx+1);
        stringStreamName << "Mute beam " << (beamIdx+1);
        addParameter(muteBeam[beamIdx] = new AudioParameterBool(stringStreamTag.str(),
                                                                 stringStreamName.str(),
                                                                 false));
        
    }
    
    stringStreamTag.str(std::string());
    stringStreamName.str(std::string());
    stringStreamTag << "gainMic";
    stringStreamName << "Gain mic";
    addParameter(micGain = new AudioParameterFloat(stringStreamTag.str(),
                                                              stringStreamName.str(),
                                                              0.0f,
                                                              40.0f,
                                                              20.0f));
    
    stringStreamTag.str(std::string());
    stringStreamName.str(std::string());
    stringStreamTag << "hpf";
    stringStreamName << "HPF mic";
    addParameter(hpfFreq = new AudioParameterFloat(stringStreamTag.str(),
                                                   stringStreamName.str(),
                                                   20.0f,
                                                   500.0f,
                                                   250.0f));
}

JucebeamAudioProcessor::~JucebeamAudioProcessor()
{
}

//==============================================================================
const String JucebeamAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool JucebeamAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool JucebeamAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool JucebeamAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double JucebeamAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int JucebeamAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int JucebeamAudioProcessor::getCurrentProgram()
{
    return 0;
}

void JucebeamAudioProcessor::setCurrentProgram (int index)
{
}

const String JucebeamAudioProcessor::getProgramName (int index)
{
    return {};
}

void JucebeamAudioProcessor::changeProgramName (int index, const String& newName)
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool JucebeamAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const{
    
    int numInputChannels = layouts.getNumChannels(true,0);
    int numOutputChannels = layouts.getNumChannels(false, 0);
    if( (numInputChannels >= 2) && (numInputChannels <= 16) && (numOutputChannels >= 2) && (numOutputChannels <= 16) ){
        /* Even though the plug-in only generates two audio outputs,
         some hosts - like Reaper - require the number of input channels to be equal to the number of output channels*/
        return true;
    }
    return false;
}
#endif

//==============================================================================
void JucebeamAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock_)
{
    
    int numInputChannels = getTotalNumInputChannels();
    
    // Initialize vMimoProcessor
    samplesPerBlock = samplesPerBlock_;
    mimoProcessor = std::make_unique<MimoProcessor>(samplesPerBlock);
    numSteeringDirections = mimoProcessor->getNumSteeringFir();
    numBeamwidthChoices = mimoProcessor->getNumBeamwidthFir();
    beamsBuffer = AudioBuffer<float>(NUM_BEAMS,mimoProcessor->fft->getSize());
    {
        GenericScopedLock<SpinLock> lock(fftInputLock);
        inputsFFT = FIR::AudioBufferFFT(numInputChannels,mimoProcessor->fft);
    }
    
    // Initialize gain ramps
    commonGain.reset();
    dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = numInputChannels;
    commonGain.prepare(spec);
    commonGain.setGainDecibels(0);
    commonGain.setRampDurationSeconds(0.1);
    
    spec.numChannels = 1;
    for (auto beamIdx = 0; beamIdx < NUM_BEAMS; ++beamIdx){
        beamGain[beamIdx].reset();
        beamGain[beamIdx].prepare(spec);
        beamGain[beamIdx].setGainDecibels(0);
        beamGain[beamIdx].setRampDurationSeconds(0.1);
    }
    
    // Initialize HPF
    iirHPFfilters.clear();
    iirCoeffHPF = IIRCoefficients::makeHighPass(getSampleRate(), hpfFreq->get());
    
    iirHPFfilters.resize(numInputChannels);
    for (auto idx = 0; idx < numInputChannels; ++idx)
    {
        iirHPFfilters[idx] = std::make_unique<IIRFilter>();
        iirHPFfilters[idx]->setCoefficients(iirCoeffHPF);
    }
    
    // Meters
    inputMeterDecay = std::make_unique<MeterDecay>(sampleRate,METERS_DECAY,samplesPerBlock,numInputChannels);
    inputMeters.resize(numInputChannels);
    beamMeterDecay = std::make_unique<MeterDecay>(sampleRate,METERS_DECAY,samplesPerBlock,NUM_BEAMS);
    beamMeters.resize(NUM_BEAMS);

}

void JucebeamAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    iirHPFfilters.clear();
    
}

void JucebeamAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    
    // HPF filtering
    if(prevHpfFreq != hpfFreq->get()){
        iirCoeffHPF = IIRCoefficients::makeHighPass(getSampleRate(), hpfFreq->get());
        prevHpfFreq = hpfFreq->get();
        for (auto idx = 0; idx < totalNumInputChannels; ++idx)
        {
            iirHPFfilters[idx]->setCoefficients(iirCoeffHPF);
        }
    }
    for (int inChannel = 0; inChannel < totalNumInputChannels; ++inChannel)
    {
        iirHPFfilters[inChannel]->processSamples(buffer.getWritePointer(inChannel), samplesPerBlock);
    }
    
    // Mic Gain
    commonGain.setGainDecibels(micGain->get());
    {
        auto block = juce::dsp::AudioBlock<float> (buffer);
        auto contextToUse = juce::dsp::ProcessContextReplacing<float> (block);
        commonGain.process(contextToUse);
    }
    
    // Mic meter
    inputMeterDecay->push(buffer);
    {
        GenericScopedLock<SpinLock> lock(inputMetersLock);
        inputMeters = inputMeterDecay->get();
    }
    
    // Compute and store fft for all input channels, so that DOA thread can operate
    {
        GenericScopedLock<SpinLock> lock(fftInputLock);
        inputsFFT.setTimeSeries(buffer);
        inputsFFT.prepareForConvolution();
        newFftInputDataAvailable = true;
    }
    
    // Per input-channel processing
    mimoProcessor->newBlock(beamsBuffer);
    for (int inChannel = 0; inChannel < totalNumInputChannels; ++inChannel)
    {
        // Beam dependent processing
        for (int beamIdx = 0; beamIdx < NUM_BEAMS; ++beamIdx)
        {
            // Determine steering index
            int steeringIdx = roundToInt(((steeringBeam[beamIdx]->get() + 1)/2.)*(numSteeringDirections-1));
            // Determine beam width index
            int beamWidthIdx = roundToInt(widthBeam[beamIdx]->get()*(numBeamwidthChoices-1));
            
            mimoProcessor->processBlock(inputsFFT,inChannel,beamsBuffer,beamIdx,steeringIdx,beamWidthIdx);
        }
    }
    
    // Beam gain
    {
        for (auto beamIdx = 0; beamIdx < NUM_BEAMS; ++beamIdx)
        {
            auto block = dsp::AudioBlock<float>(beamsBuffer);
            beamGain[beamIdx].setGainDecibels(levelBeam[beamIdx]->get());
            {
                auto beamBlock = block.getSubsetChannelBlock(beamIdx, 1).getSubBlock(0, samplesPerBlock);
                auto contextToUse = dsp::ProcessContextReplacing<float> (beamBlock);
                beamGain[beamIdx].process(contextToUse);
            }
        }
    }
    
    // Beam meters
    beamMeterDecay->push(beamsBuffer);
    {
        GenericScopedLock<SpinLock> lock(beamMetersLock);
        beamMeters = beamMeterDecay->get();
    }
    
    // Sum beams in output channels
    for (int outChannel = 0; outChannel < OUT_CHANNELS; ++outChannel)
    {
        // Clean output buffer
        buffer.clear(outChannel,0,samplesPerBlock);
        // Sum the contributes from each beam
        for (int beamIdx = 0; beamIdx < NUM_BEAMS; ++beamIdx){
            if (muteBeam[beamIdx]->get() == false){
                float channelBeamGain = panToLinearGain(panBeam[beamIdx],outChannel==0);
                // Add to buffer
                buffer.addFrom(outChannel, 0, beamsBuffer, beamIdx, 0, samplesPerBlock, channelBeamGain);
            }
        }
    }
    for (int outChannel = OUT_CHANNELS; outChannel < buffer.getNumChannels(); ++outChannel){
        buffer.clear(outChannel,0,samplesPerBlock);
    }
    
}

//==============================================================================
bool JucebeamAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* JucebeamAudioProcessor::createEditor()
{
    return new JucebeamAudioProcessorEditor (*this);
}

//==============================================================================
void JucebeamAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    std::ostringstream stringStreamTag;
    std::unique_ptr<XmlElement> xml (new XmlElement ("eBeamer"));
    for (uint8 beamIdx = 0; beamIdx < NUM_BEAMS; ++beamIdx){
        stringStreamTag.str(std::string());
        stringStreamTag << "steerBeam" << (beamIdx+1);
        xml->setAttribute(Identifier(stringStreamTag.str()), (double) *(steeringBeam[beamIdx]));
        
        stringStreamTag.str(std::string());
        stringStreamTag << "widthBeam" << (beamIdx+1);
        xml->setAttribute(Identifier(stringStreamTag.str()), (double) *(widthBeam[beamIdx]));
        
        stringStreamTag.str(std::string());
        stringStreamTag << "panBeam" << (beamIdx+1);
        xml->setAttribute(Identifier(stringStreamTag.str()), (double) *(panBeam[beamIdx]));
        
        stringStreamTag.str(std::string());
        stringStreamTag << "levelBeam" << (beamIdx+1);
        xml->setAttribute(Identifier(stringStreamTag.str()), (double) *(levelBeam[beamIdx]));

        stringStreamTag.str(std::string());
        stringStreamTag << "muteBeam" << (beamIdx+1);
        xml->setAttribute(Identifier(stringStreamTag.str()), (bool) *(muteBeam[beamIdx]));
        
    }
    stringStreamTag.str(std::string());
    stringStreamTag << "hpfFreq";
    xml->setAttribute(Identifier(stringStreamTag.str()), (double) *(hpfFreq));
    
    stringStreamTag.str(std::string());
    stringStreamTag << "gainMic";
    xml->setAttribute(Identifier(stringStreamTag.str()), (double) *(micGain));
    
    copyXmlToBinary (*xml, destData);
}

void JucebeamAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::ostringstream stringStreamTag;
    std::unique_ptr<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr){
        if (xmlState->hasTagName ("eBeamer")){
            for (uint8 beamIdx = 0; beamIdx < NUM_BEAMS; ++beamIdx){
                stringStreamTag.str(std::string());
                stringStreamTag << "steerBeam" << (beamIdx+1);
                *(steeringBeam[beamIdx]) = xmlState->getDoubleAttribute (Identifier(stringStreamTag.str()), 0.0);
                
                stringStreamTag.str(std::string());
                stringStreamTag << "widthBeam" << (beamIdx+1);
                *(widthBeam[beamIdx]) = xmlState->getDoubleAttribute (Identifier(stringStreamTag.str()), 0.0);
                
                stringStreamTag.str(std::string());
                stringStreamTag << "panBeam" << (beamIdx+1);
                *(panBeam[beamIdx]) = xmlState->getDoubleAttribute (Identifier(stringStreamTag.str()), 0.0);
                
                stringStreamTag.str(std::string());
                stringStreamTag << "levelBeam" << (beamIdx+1);
                *(levelBeam[beamIdx]) = xmlState->getDoubleAttribute (Identifier(stringStreamTag.str()), 0.0);
                
                stringStreamTag.str(std::string());
                stringStreamTag << "muteBeam" << (beamIdx+1);
                *(muteBeam[beamIdx]) = xmlState->getBoolAttribute(Identifier(stringStreamTag.str()), false);
                
            }
            stringStreamTag.str(std::string());
            stringStreamTag << "hpfFreq";
            *(hpfFreq) = xmlState->getDoubleAttribute (Identifier(stringStreamTag.str()), 250.0);
            
            stringStreamTag.str(std::string());
            stringStreamTag << "gainMic";
            *(micGain) = xmlState->getDoubleAttribute (Identifier(stringStreamTag.str()), 20.0);
        }
    }
}


//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new JucebeamAudioProcessor();
}

//==============================================================================
// Meters

float JucebeamAudioProcessor::getBeamMeter(int channel){
    GenericScopedLock<SpinLock> lock(beamMetersLock);
    return beamMeters[channel];
}

std::vector<float> JucebeamAudioProcessor::getInputMeters(){
    GenericScopedLock<SpinLock> lock(inputMetersLock);
    return inputMeters;
}
