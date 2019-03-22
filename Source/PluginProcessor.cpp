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
    
    // Initialize FFT
    fft = std::make_unique<dsp::FFT>(roundToInt (std::log2 (FFT_SIZE)));
    
    // Initialize firFFTs (already prepared for convolution
#ifdef BEAMSTEERING_ALG_IDEAL
    firFFT = prepareIR(readFIR(firIR::firDASideal_dat,firIR::firDASideal_datSize));
#else
    firFFT = prepareIR(readFIR(firIR::firDASmeasured_dat,firIR::firDASmeasured_datSize));
#endif
    /* With Joe we decided that the way we want the interface to behave is to have
     the eStick facing the user, mic 1 on the left, thus we have to reverse the order of the filters.
     */
    std::reverse(firFFT.begin(), firFFT.end());
    firBeamwidthFft = prepareIR(readFIR(firIR::firBeamwidth_dat,firIR::firBeamwidth_datSize));
    
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
    
    auto bufLen = std::max(FFT_SIZE,samplesPerBlock+(FFT_SIZE - MAX_FFT_BLOCK_LEN));
    beamBuffer = AudioBuffer<float>(getTotalNumOutputChannels(),bufLen);
    beamBuffer.clear();
    
    fftInput = AudioBuffer<float>(1,2*FFT_SIZE);
    fftBuffer = AudioBuffer<float>(1,2*FFT_SIZE);
    fftOutput = AudioBuffer<float>(1,2*FFT_SIZE);
    fftInput.clear();
    fftBuffer.clear();
    fftOutput.clear();
    
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
    inputMeters = std::vector<float>(numInputChannels);
    beamMeterDecay = std::make_unique<MeterDecay>(sampleRate,METERS_DECAY,samplesPerBlock,NUM_BEAMS);
    beamMeters = std::vector<float>(NUM_BEAMS);
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
    
    commonGain.setGainDecibels(micGain->get());
    {
        auto block = juce::dsp::AudioBlock<float> (buffer);
        auto contextToUse = juce::dsp::ProcessContextReplacing<float> (block);
        commonGain.process(contextToUse);
    }
    
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
        // HPF filtering
        iirHPFfilters[inChannel]->processSamples(buffer.getWritePointer(inChannel), blockNumSamples);
    }
    
    // Meter
    inputMeterDecay->push(buffer);
    inputMetersLock.enter();
    inputMeters = inputMeterDecay->get();
    inputMetersLock.exit();
    
    for (int inChannel = 0; inChannel < totalNumInputChannels; ++inChannel)
    {
        
        for (auto subBlockIdx = 0;subBlockIdx < std::ceil(float(blockNumSamples)/MAX_FFT_BLOCK_LEN);++subBlockIdx)
        {
            auto subBlockFirstIdx = subBlockIdx * MAX_FFT_BLOCK_LEN;
            auto subBlockLen = std::min(blockNumSamples - subBlockFirstIdx,MAX_FFT_BLOCK_LEN);
            
            // Fill fft data buffer
            fftInput.clear();
            fftInput.copyFrom(0, 0, buffer, inChannel, subBlockFirstIdx, subBlockLen);
            
            // Forward channel FFT
            fft -> performRealOnlyForwardTransform(fftInput.getWritePointer(0));
            
            // Push FFT data for DOAthread to retrieve
            const GenericScopedLock<SpinLock> scopedLock(fftLock);
            
            pushBackFFTdata(fftInput, inChannel);
            
            const GenericScopedUnlock<SpinLock> scopedUnlock(fftLock);
            
            // Beam dependent processing
            for (int beamIdx = 0; beamIdx < NUM_BEAMS; ++beamIdx)
            {
                
                fftBuffer.copyFrom(0, 0, fftInput, 0, 0, fftInput.getNumSamples());
                
                // Determine steering index
                int steeringIdx = roundToInt(((steeringBeam[beamIdx]->get() + 1)/2.)*(firFFT.size()-1));
                
                // Determine beam width index
                int beamWidthIdx = roundToInt(widthBeam[beamIdx]->get()*(firBeamwidthFft.size()-1));
                
                // FIR pre processing
                prepareForConvolution(fftBuffer.getWritePointer(0));
                
                // Beam width processing
                fftOutput.clear();
                convolutionProcessingAndAccumulate(fftBuffer.getReadPointer(0),firBeamwidthFft[beamWidthIdx][inChannel].data(),fftOutput.getWritePointer(0));
                
                // Beam steering processing
                fftBuffer.copyFrom(0, 0, fftOutput, 0, 0, fftOutput.getNumSamples());
                fftOutput.clear();
                convolutionProcessingAndAccumulate(fftBuffer.getReadPointer(0),firFFT[steeringIdx][inChannel].data(),fftOutput.getWritePointer(0));
                
                // FIR post processing
                updateSymmetricFrequencyDomainData(fftOutput.getWritePointer(0));
                
                // Inverse FFT
                fft -> performRealOnlyInverseTransform(fftOutput.getWritePointer(0));
                
                // OLA
                beamBuffer.addFrom(beamIdx, subBlockFirstIdx, fftOutput.getReadPointer(0), FFT_SIZE);
                
            }
        }
    }
    
    // Gain
    {
        
        for (auto beamIdx = 0; beamIdx < NUM_BEAMS; ++beamIdx)
        {
            auto block = juce::dsp::AudioBlock<float> (beamBuffer);
            beamGain[beamIdx].setGainDecibels(levelBeam[beamIdx]->get());
            {
                auto beamBlock = block.getSubsetChannelBlock(beamIdx, 1).getSubBlock(0, blockNumSamples);
                auto contextToUse = juce::dsp::ProcessContextReplacing<float> (beamBlock);
                beamGain[beamIdx].process(contextToUse);
            }
        }
    }
    
    beamMeterDecay->push(beamBuffer);
    beamMetersLock.enter();
    beamMeters = beamMeterDecay->get();
    beamMetersLock.exit();
    
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
                buffer.addFrom(outChannel, 0, beamBuffer, beamIdx, 0, blockNumSamples, channelBeamGain);
            }
        }
    }
    
    // Shift beam OLA buffer
    for (int beamIdx = 0; beamIdx < NUM_BEAMS; ++beamIdx){
        FloatVectorOperations::copy(beamBuffer.getWritePointer(beamIdx), &(beamBuffer.getReadPointer(beamIdx)[blockNumSamples]), beamBuffer.getNumSamples()-blockNumSamples);
        beamBuffer.clear(beamIdx, beamBuffer.getNumSamples()-blockNumSamples, blockNumSamples);
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
        stringStreamTag << "gainBeam" << (beamIdx+1);
        xml->setAttribute(Identifier(stringStreamTag.str()), (double) *(levelBeam[beamIdx]));

        stringStreamTag.str(std::string());
        stringStreamTag << "muteBeam" << (beamIdx+1);
        xml->setAttribute(Identifier(stringStreamTag.str()), (bool) *(muteBeam[beamIdx]));
    }
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
                stringStreamTag << "gainBeam" << (beamIdx+1);
                *(levelBeam[beamIdx]) = xmlState->getDoubleAttribute (Identifier(stringStreamTag.str()), 10.0);
                
                stringStreamTag.str(std::string());
                stringStreamTag << "muteBeam" << (beamIdx+1);
                *(muteBeam[beamIdx]) = xmlState->getBoolAttribute(Identifier(stringStreamTag.str()), false);
            }
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

void JucebeamAudioProcessor::firConvolve(float *input, float *output, int inChannel, int beamWidthIdx, int steeringIdx)
{
    float fftTemp[2*FFT_SIZE];
    
    FloatVectorOperations::copy(fftTemp, fftInput, 2*FFT_SIZE);
    
    // FIR pre processing
    prepareForConvolution(fftTemp);
    
    // Beam width processing
    FloatVectorOperations::clear(output, 2*FFT_SIZE);
    convolutionProcessingAndAccumulate(fftTemp, firBeamwidthFft[beamWidthIdx][inChannel].data(), output);
    
    // Beam steering processing
    FloatVectorOperations::copy(fftTemp, output, 2*FFT_SIZE);
    FloatVectorOperations::clear(output, 2*FFT_SIZE);
    convolutionProcessingAndAccumulate(fftTemp, firFFT[steeringIdx][inChannel].data(), output);
    
    // FIR post processing
    updateSymmetricFrequencyDomainData(output);
    
    // Inverse FFT
    fft -> performRealOnlyInverseTransform(output);
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
            std::vector<float> firFFTAngleMic(2*FFT_SIZE);
            FloatVectorOperations::clear(firFFTAngleMic.data(), 2*FFT_SIZE);
            FloatVectorOperations::copy(firFFTAngleMic.data(), fir[angleIdx][micIdx].data() , static_cast<int>(fir[angleIdx][micIdx].size()));
            fft -> performRealOnlyForwardTransform(firFFTAngleMic.data());
            prepareForConvolution(firFFTAngleMic.data());
            firFFTAngle [micIdx] = firFFTAngleMic;
        }
        firFFT[angleIdx] = firFFTAngle;
    }
    
    return firFFT;
}

//========== copied from juce_Convolution.cpp ============

/** After each FFT, this function is called to allow convolution to be performed with only 4 SIMD functions calls. */
void JucebeamAudioProcessor::prepareForConvolution (float *samples) noexcept
{
    auto FFTSizeDiv2 = FFT_SIZE / 2;
    
    for (size_t i = 0; i < FFTSizeDiv2; i++)
        samples[i] = samples[2 * i];
    
    samples[FFTSizeDiv2] = 0;
    
    for (size_t i = 1; i < FFTSizeDiv2; i++)
        samples[i + FFTSizeDiv2] = -samples[2 * (FFT_SIZE - i) + 1];
}

/** Does the convolution operation itself only on half of the frequency domain samples. */
void JucebeamAudioProcessor::convolutionProcessingAndAccumulate (const float *input, const float *impulse, float *output)
{
    auto FFTSizeDiv2 = FFT_SIZE / 2;
    
    FloatVectorOperations::addWithMultiply      (output, input, impulse, static_cast<int> (FFTSizeDiv2));
    FloatVectorOperations::subtractWithMultiply (output, &(input[FFTSizeDiv2]), &(impulse[FFTSizeDiv2]), static_cast<int> (FFTSizeDiv2));
    
    FloatVectorOperations::addWithMultiply      (&(output[FFTSizeDiv2]), input, &(impulse[FFTSizeDiv2]), static_cast<int> (FFTSizeDiv2));
    FloatVectorOperations::addWithMultiply      (&(output[FFTSizeDiv2]), &(input[FFTSizeDiv2]), impulse, static_cast<int> (FFTSizeDiv2));
    
    output[FFT_SIZE] += input[FFT_SIZE] * impulse[FFT_SIZE];
}

/** Undo the re-organization of samples from the function prepareForConvolution.
 Then, takes the conjugate of the frequency domain first half of samples, to fill the
 second half, so that the inverse transform will return real samples in the time domain.
 */
void JucebeamAudioProcessor::updateSymmetricFrequencyDomainData (float* samples) noexcept
{
    auto FFTSizeDiv2 = FFT_SIZE / 2;
    
    for (size_t i = 1; i < FFTSizeDiv2; i++)
    {
        samples[2 * (FFT_SIZE - i)] = samples[i];
        samples[2 * (FFT_SIZE - i) + 1] = -samples[FFTSizeDiv2 + i];
    }
    
    samples[1] = 0.f;
    
    for (size_t i = 1; i < FFTSizeDiv2; i++)
    {
        samples[2 * i] = samples[2 * (FFT_SIZE - i)];
        samples[2 * i + 1] = -samples[2 * (FFT_SIZE - i) + 1];
    }
}

int JucebeamAudioProcessor::bufferStatus()
{
    if(fftData.size() != getTotalNumInputChannels())
        return 100; // The buffer is completely empty (no pushes yet)
        
    int cursor = BUFFER_UPPER_THRESHOLD;
    for(int i = 0; i < getTotalNumInputChannels(); i++)
        if(cursor > fftData.at(i).size())
            cursor = fftData.at(i).size();
    
    if(cursor == BUFFER_UPPER_THRESHOLD)
        return (cursor - BUFFER_UPPER_THRESHOLD);
        
    if(cursor < BUFFER_LOWER_THRESHOLD)
        return (cursor - BUFFER_LOWER_THRESHOLD);
        
    return 0;
}

std::vector<float*> JucebeamAudioProcessor::popFrontFFTdata()
{
    std::vector<float*> result;
    
    if(fftData.size() != getTotalNumInputChannels())
        return result;
    
    for(int i = 0; i < getTotalNumInputChannels(); i++){
        if(fftData.at(i).size() == 0){
            // The first entry of the buffer hasn't been filled for all channels yet.
            result.clear();
            return result;
        }
        
        result.push_back(fftData.at(i).front());
    }
    
    // If we reached this point, the first entry was full and result is ready.
    for(int i = 0; i < getTotalNumInputChannels(); i++)
        fftData.at(i).erase(fftData.at(i).begin());
    
    return result;
}

void JucebeamAudioProcessor::pushBackFFTdata(float* input, int channelIdx)
{
    // Assuming the number of input channels won't change at runtime.
    
    if(fftData.size() != getTotalNumInputChannels())
        fftData.resize(getTotalNumInputChannels());
    
    if(channelIdx < 0 || channelIdx > getTotalNumInputChannels())
        return;
    
    fftData.at(channelIdx).push_back(input);
}

