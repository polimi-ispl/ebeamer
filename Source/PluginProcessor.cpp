#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Binary/firIR.h"

static float panToLinearGain(const AudioParameterFloat* gain, const bool isLeftChannel) {
    const float db_at0 = -4.5; //How many dB at each channel when pan is centered (0)
    float gainParam = gain->get();
    jassert(gainParam >= -1);
    jassert(gainParam <= 1);
    float alpha = std::pow(10.,(db_at0/20.));
    if (isLeftChannel){
        gainParam = -gainParam;
    }
    float y = (0.5-alpha)*std::pow(gainParam,2.)+0.5*gainParam+alpha;
    return y;
}

static std::vector<std::vector<std::vector<float> > > readFIR(const char* array,const int len) {
    MemoryInputStream inputStream(array, len,false);
    uint32 numFilters;
    inputStream.read(&numFilters, 4);
    uint32 numChannels;
    inputStream.read(&numChannels, 4);
    uint32 filtersLen;
    inputStream.read(&filtersLen, 4);
    uint32 fs;
    inputStream.read(&fs, 4);
    
    std::vector<std::vector<std::vector<float>>> fir(numFilters);
    for (auto filterIdx = 0; filterIdx < numFilters; ++filterIdx){
        fir[filterIdx].resize(numChannels);
        for (auto channelIdx = 0; channelIdx < numChannels; ++channelIdx){
            fir[filterIdx][channelIdx].resize(filtersLen);
            for (auto coeffIdx = 0; coeffIdx < filtersLen; ++coeffIdx){
                inputStream.read(&(fir[filterIdx][channelIdx][coeffIdx]),4);
            }
        }
    }
    return fir;
}

//==============================================================================
JucebeamAudioProcessor::JucebeamAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                       .withInput  ("Input",  AudioChannelSet::ambisonic(3), true)
                       /*.withInput  ("Input",  AudioChannelSet::stereo(), true)*/
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
    if( (numInputChannels >= 2) && (numInputChannels <= 16) && (numOutputChannels == 2) ){
        return true;
    }
    return false;
}
#endif

//==============================================================================
void JucebeamAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    
    auto numInputChannels = getTotalNumInputChannels();
    
    size_t fftOrder = roundToInt (std::log2 (FIR_LEN + samplesPerBlock - 1));
    
    // Initialize FFT
    fft = std::make_unique<dsp::FFT>(fftOrder);
    
    // Initialize firFFTs (already prepared for convolution)
#ifdef BEAMSTEERING_ALG_IDEAL
    firSteeringFFT = prepareIR(readFIR(firIR::firDASideal_dat,firIR::firDASideal_datSize));
#else
    firFFT = prepareIR(readFIR(firIR::firDASmeasured_dat,firIR::firDASmeasured_datSize));
#endif
    /* With Joe we decided that the way we want the interface to behave is to have
     the eStick facing the user, mic 1 on the left, thus we have to reverse the order of the filters.
     */
    std::reverse(firSteeringFFT.begin(), firSteeringFFT.end());
    firBeamwidthFFT = prepareIR(readFIR(firIR::firBeamwidth_dat,firIR::firBeamwidth_datSize));
    
    // Allocate beams output buffers
    beamsBuffer = AudioBuffer<float>(getTotalNumOutputChannels(),getFftSize());
    beamsBuffer.clear();
    
    // Allocate single channel buffers
    fftInputLock.enter();
    fftInput = AudioBuffer<float>(numInputChannels,2*getFftSize());
    fftInput.clear();
    fftInputLock.exit();
    
    fftBuffer = AudioBuffer<float>(1,2*getFftSize());
    fftBuffer.clear();
    
    fftOutput = AudioBuffer<float>(1,2*getFftSize());
    fftOutput.clear();
    
    
    // Initialize gain ramps
    commonGain.reset();
    dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = getTotalNumOutputChannels();
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
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    int blockNumSamples = buffer.getNumSamples();
    
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
        iirHPFfilters[inChannel]->processSamples(buffer.getWritePointer(inChannel), blockNumSamples);
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
    fftInputLock.enter();
    fftInput.clear();
    for (int inChannel = 0; inChannel < totalNumInputChannels; ++inChannel)
    {
        fftInput.copyFrom(inChannel, 0, buffer, inChannel, 0, blockNumSamples);
        fft -> performRealOnlyForwardTransform(fftInput.getWritePointer(inChannel));
        prepareForConvolution(fftInput.getWritePointer(inChannel),getFftSize());
    }
    newFftInputDataAvailable = true;
    fftInputLock.exit();
    
    // Per input-channel processing
    for (int inChannel = 0; inChannel < totalNumInputChannels; ++inChannel)
    {
        
        // Beam dependent processing
        for (int beamIdx = 0; beamIdx < NUM_BEAMS; ++beamIdx)
        {
            fftBuffer.copyFrom(0, 0, fftInput, inChannel, 0, fftInput.getNumSamples());
            
            // Determine steering index
            int steeringIdx = roundToInt(((steeringBeam[beamIdx]->get() + 1)/2.)*(firSteeringFFT.size()-1));
            
            // Determine beam width index
            int beamWidthIdx = roundToInt(widthBeam[beamIdx]->get()*(firBeamwidthFFT.size()-1));
            
            // Beam width processing
            fftOutput.clear();
            convolutionProcessingAndAccumulate(fftBuffer.getReadPointer(0),firBeamwidthFFT[beamWidthIdx][inChannel].data(),fftOutput.getWritePointer(0),getFftSize());
            
            // Beam steering processing
            fftBuffer.copyFrom(0, 0, fftOutput, 0, 0, fftOutput.getNumSamples());
            fftOutput.clear();
            convolutionProcessingAndAccumulate(fftBuffer.getReadPointer(0),firSteeringFFT[steeringIdx][inChannel].data(),fftOutput.getWritePointer(0),getFftSize());
            
            // FIR post processing
            updateSymmetricFrequencyDomainData(fftOutput.getWritePointer(0),getFftSize());
            
            // Inverse FFT
            fft -> performRealOnlyInverseTransform(fftOutput.getWritePointer(0));
            
            // OLA
            beamsBuffer.addFrom(beamIdx, 0, fftOutput.getReadPointer(0), getFftSize());
                
        }
    }
    
    // Beam gain
    {
        for (auto beamIdx = 0; beamIdx < NUM_BEAMS; ++beamIdx)
        {
            auto block = dsp::AudioBlock<float> (beamsBuffer);
            beamGain[beamIdx].setGainDecibels(levelBeam[beamIdx]->get());
            {
                auto beamBlock = block.getSubsetChannelBlock(beamIdx, 1).getSubBlock(0, blockNumSamples);
                auto contextToUse = dsp::ProcessContextReplacing<float> (beamBlock);
                beamGain[beamIdx].process(contextToUse);
            }
        }
    }
    
    beamMeterDecay->push(beamsBuffer);
    
    {
        GenericScopedLock<SpinLock> lock(beamMetersLock);
        beamMeters = beamMeterDecay->get();
    }
    
    // Sum beams in output channels
    for (int outChannel = 0; outChannel < totalNumOutputChannels; ++outChannel)
    {
        // Clean output buffer
        buffer.clear(outChannel,0,blockNumSamples);
        // Sum the contributes from each beam
        for (int beamIdx = 0; beamIdx < NUM_BEAMS; ++beamIdx){
            if (muteBeam[beamIdx]->get() == false){
                float channelBeamGain = panToLinearGain(panBeam[beamIdx],outChannel==0);
                // Add to buffer
                buffer.addFrom(outChannel, 0, beamsBuffer, beamIdx, 0, blockNumSamples, channelBeamGain);
            }
        }
    }
    
    // Shift beam OLA buffer
    for (int beamIdx = 0; beamIdx < NUM_BEAMS; ++beamIdx){
        FloatVectorOperations::copy(beamsBuffer.getWritePointer(beamIdx), &(beamsBuffer.getReadPointer(beamIdx)[blockNumSamples]), beamsBuffer.getNumSamples()-blockNumSamples);
        beamsBuffer.clear(beamIdx, beamsBuffer.getNumSamples()-blockNumSamples, blockNumSamples);
    }
}

//==============================================================================
bool JucebeamAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* JucebeamAudioProcessor::createEditor()
{
    editor = new JucebeamAudioProcessorEditor (*this);
    return editor;
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

//=======================================================
std::vector<std::vector<std::vector<float>>> JucebeamAudioProcessor::prepareIR(const std::vector<std::vector<std::vector<float>>> fir)
{
    std::vector<std::vector<std::vector<float>>> firFFT(fir.size());
    for (size_t angleIdx = 0; angleIdx < fir.size(); ++angleIdx)
    {
        std::vector<std::vector<float>> firFFTAngle(fir[angleIdx].size());
        for (size_t micIdx = 0; micIdx < fir[angleIdx].size(); ++micIdx)
        {
            std::vector<float> firFFTAngleMic(2*getFftSize());
            FloatVectorOperations::clear(firFFTAngleMic.data(), 2*getFftSize());
            FloatVectorOperations::copy(firFFTAngleMic.data(), fir[angleIdx][micIdx].data() , static_cast<int>(fir[angleIdx][micIdx].size()));
            fft -> performRealOnlyForwardTransform(firFFTAngleMic.data());
            prepareForConvolution(firFFTAngleMic.data(),getFftSize());
            firFFTAngle [micIdx] = firFFTAngleMic;
        }
        firFFT[angleIdx] = firFFTAngle;
    }
    
    return firFFT;
}

//========== copied from juce_Convolution.cpp ============

/** After each FFT, this function is called to allow convolution to be performed with only 4 SIMD functions calls. */
void JucebeamAudioProcessor::prepareForConvolution (float *samples, int fftSize) noexcept
{
    auto FFTSizeDiv2 = fftSize / 2;
    
    for (size_t i = 0; i < FFTSizeDiv2; i++)
        samples[i] = samples[2 * i];
    
    samples[FFTSizeDiv2] = 0;
    
    for (size_t i = 1; i < FFTSizeDiv2; i++)
        samples[i + FFTSizeDiv2] = -samples[2 * (fftSize - i) + 1];
}

/** Does the convolution operation itself only on half of the frequency domain samples. */
void JucebeamAudioProcessor::convolutionProcessingAndAccumulate (const float *input, const float *impulse, float *output, int fftSize)
{
    auto FFTSizeDiv2 = fftSize / 2;
    
    FloatVectorOperations::addWithMultiply      (output, input, impulse, static_cast<int> (FFTSizeDiv2));
    FloatVectorOperations::subtractWithMultiply (output, &(input[FFTSizeDiv2]), &(impulse[FFTSizeDiv2]), static_cast<int> (FFTSizeDiv2));
    
    FloatVectorOperations::addWithMultiply      (&(output[FFTSizeDiv2]), input, &(impulse[FFTSizeDiv2]), static_cast<int> (FFTSizeDiv2));
    FloatVectorOperations::addWithMultiply      (&(output[FFTSizeDiv2]), &(input[FFTSizeDiv2]), impulse, static_cast<int> (FFTSizeDiv2));
    
    output[fftSize] += input[fftSize] * impulse[fftSize];
}

/** Undo the re-organization of samples from the function prepareForConvolution.
 Then, takes the conjugate of the frequency domain first half of samples, to fill the
 second half, so that the inverse transform will return real samples in the time domain.
 */
void JucebeamAudioProcessor::updateSymmetricFrequencyDomainData (float* samples, int fftSize) noexcept
{
    auto FFTSizeDiv2 = fftSize / 2;
    
    for (size_t i = 1; i < FFTSizeDiv2; i++)
    {
        samples[2 * (fftSize - i)] = samples[i];
        samples[2 * (fftSize - i) + 1] = -samples[FFTSizeDiv2 + i];
    }
    
    samples[1] = 0.f;
    
    for (size_t i = 1; i < FFTSizeDiv2; i++)
    {
        samples[2 * i] = samples[2 * (fftSize - i)];
        samples[2 * i + 1] = -samples[2 * (fftSize - i) + 1];
    }
}


AudioBuffer<float> JucebeamAudioProcessor::waitGetNewFFTinput(){
    
    while (! newFftInputDataAvailable){
        sleep(0.01);
    }
    
    AudioBuffer<float> fftInputCopy;
    
    fftInputLock.enter();
    fftInputCopy.makeCopyOf(fftInput);
    newFftInputDataAvailable = false;
    fftInputLock.exit();
    
    return fftInputCopy;
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
