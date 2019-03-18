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
    firBeamwidthFft = prepareIR(readFIR(firIR::firBeamwidth_dat,firIR::firBeamwidth_datSize));
    
    // Initialize parameters
    std::ostringstream stringStreamTag;
    std::ostringstream stringStreamName;
    for (uint8 beamIdx = 0; beamIdx < NUM_BEAMS; ++beamIdx){
        stringStreamTag.str(std::string());
        stringStreamName.str(std::string());
        stringStreamTag << "steerBeam" << (beamIdx+1);
        stringStreamName << "Steering beam " << (beamIdx+1);
        addParameter(steeringBeam[beamIdx] = new AudioParameterFloat(stringStreamTag.str(),
                                                               stringStreamName.str(),
                                                               -1.0f,
                                                               1.0f,
                                                               -0.2f));
        stringStreamTag.str(std::string());
        stringStreamName.str(std::string());
        stringStreamTag << "widthBeam" << (beamIdx+1);
        stringStreamName << "Width beam " << (beamIdx+1);
        addParameter(widthBeam[beamIdx] = new AudioParameterFloat(stringStreamTag.str(),
                                                                  stringStreamName.str(),
                                                            0.0f,
                                                            1.0f,
                                                            0.0f));
        stringStreamTag.str(std::string());
        stringStreamName.str(std::string());
        stringStreamTag << "panBeam" << (beamIdx+1);
        stringStreamName << "Pan beam " << (beamIdx+1);
        addParameter(panBeam[beamIdx] = new AudioParameterFloat(stringStreamTag.str(),
                                                                stringStreamName.str(),
                                                          -1.0f,
                                                          1.0f,
                                                          0.0f));
        stringStreamTag.str(std::string());
        stringStreamName.str(std::string());
        stringStreamTag << "gainBeam" << (beamIdx+1);
        stringStreamName << "Gain beam " << (beamIdx+1);
        addParameter(gainBeam[beamIdx] = new AudioParameterFloat(stringStreamTag.str(),
                                                                 stringStreamName.str(),
                                                           0.0f,
                                                           60.0f,
                                                           10.0f));
        
        stringStreamTag.str(std::string());
        stringStreamName.str(std::string());
        stringStreamTag << "muteBeam" << (beamIdx+1);
        stringStreamName << "Mute beam " << (beamIdx+1);
        addParameter(muteBeam[beamIdx] = new AudioParameterBool(stringStreamTag.str(),
                                                                 stringStreamName.str(),
                                                                 false));
        
    }
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
    
    // Initialize HPF
    iirHPFfilters.clear();
    iirCoeffHPF = IIRCoefficients::makeHighPass(getSampleRate(), HPF_FREQ);
    
    iirHPFfilters.resize(numInputChannels);
    for (auto idx = 0; idx < numInputChannels; ++idx)
    {
        iirHPFfilters[idx] = std::make_unique<IIRFilter>();
        iirHPFfilters[idx]->setCoefficients(iirCoeffHPF);
    }
    
    // Meters
    inputMetersLock.enter();
    inputMeters.clear();
    for (auto idx = 0; idx < numInputChannels; ++idx)
        inputMeters.push_back(0);
    inputMetersLock.exit();
    
    beamMetersLock.enter();
    beamMeters.clear();
    for (auto idx = 0; idx < NUM_BEAMS; ++idx)
        beamMeters.push_back(0);
    beamMetersLock.exit();
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
    
    float commonGaindB = 100;
    for (auto beamIdx = 0; beamIdx < NUM_BEAMS; ++beamIdx){
        commonGaindB = std::min(commonGaindB,gainBeam[beamIdx]->get());
    }
    commonGaindB -= 6;
    float commonGainLinear = Decibels::decibelsToGain(commonGaindB);
    buffer.applyGain(commonGainLinear);
    
    for (int inChannel = 0; inChannel < totalNumInputChannels; ++inChannel)
    {
        
        // HPF filtering
        iirHPFfilters[inChannel]->processSamples(buffer.getWritePointer(inChannel), blockNumSamples);
        
        // Meter
        {
            inputMetersLock.enter();
            float minVal,maxVal,maxAbsVal;
            for (auto beamIdx = 0; beamIdx < NUM_BEAMS; ++beamIdx)
            {
#ifdef METERS_MODE_RMS
                inputMeters[inChannel] = inputMeters[inChannel]*(METERS_INERTIA) + buffer.getRMSLevel(inChannel, 0, blockNumSamples)*(1-METERS_INERTIA);
#else
                findMinAndMax(buffer.getReadPointer(inChannel), blockNumSamples, minVal, maxVal);
                maxAbsVal = jmax(abs(minVal),abs(maxVal));
                inputMeters[inChannel] = inputMeters[inChannel]*(METERS_INERTIA) + maxAbsVal*(1-METERS_INERTIA);
#endif
            }
            inputMetersLock.exit();
        }
        
        for (auto subBlockIdx = 0;subBlockIdx < std::ceil(float(blockNumSamples)/MAX_FFT_BLOCK_LEN);++subBlockIdx)
        {
            auto subBlockFirstIdx = subBlockIdx * MAX_FFT_BLOCK_LEN;
            auto subBlockLen = std::min(blockNumSamples - subBlockFirstIdx,MAX_FFT_BLOCK_LEN);
            
            // Fill fft data buffer
            FloatVectorOperations::clear(fftInput, 2*FFT_SIZE);
            FloatVectorOperations::copy(fftInput, &(buffer.getReadPointer(inChannel)[subBlockFirstIdx]),subBlockLen);
            
            // Forward channel FFT
            fft -> performRealOnlyForwardTransform(fftInput);
            
            // Push FFT data for DOAthread to retrieve
            const GenericScopedLock<SpinLock> scopedLock(fftLock);
            
            pushBackFFTdata(fftInput);
            
            const GenericScopedUnlock<SpinLock> scopedUnlock(fftLock);
            
            // Beam dependent processing
            for (int beamIdx = 0; beamIdx < NUM_BEAMS; ++beamIdx)
            {
                if (bypass)
                {
                    // OLA
                    if (beamIdx == inChannel)
                    {
                        beamBuffer.addFrom(beamIdx, subBlockFirstIdx, buffer, inChannel, subBlockFirstIdx,  subBlockLen);
                    }
                }
                else
                { // No bypass
                    FloatVectorOperations::copy(fftBuffer,fftInput,2*FFT_SIZE);
                    
                    if (passThrough)
                    {
                        if (beamIdx == inChannel)
                        {
                            // Pass-through processing
                            FloatVectorOperations::copy(fftOutput, fftBuffer, 2*FFT_SIZE);
                            // Inverse FFT
                            fft -> performRealOnlyInverseTransform(fftOutput);
                            // OLA
                            beamBuffer.addFrom(beamIdx, subBlockFirstIdx, fftOutput, FFT_SIZE);
                        }
                    }
                    else{ // no passThrough, real processing here!
                        
                        // Determine steering index
                        int steeringIdx = roundToInt(((steeringBeam[beamIdx]->get() + 1)/2.)*(firFFT.size()-1));
                        
                        // Determine beam width index
                        int beamWidthIdx = roundToInt(widthBeam[beamIdx]->get()*(firBeamwidthFft.size()-1));
                        
                        // FIR pre processing
                        prepareForConvolution(fftBuffer);
                        
                        // Beam width processing
                        FloatVectorOperations::clear(fftOutput, 2*FFT_SIZE);
                        convolutionProcessingAndAccumulate(fftBuffer,firBeamwidthFft[beamWidthIdx][inChannel].data(),fftOutput);
                        
                        // Beam steering processing
                        FloatVectorOperations::copy(fftBuffer, fftOutput, 2*FFT_SIZE);
                        FloatVectorOperations::clear(fftOutput, 2*FFT_SIZE);
                        convolutionProcessingAndAccumulate(fftBuffer,firFFT[steeringIdx][inChannel].data(),fftOutput);
                        
                        // FIR post processing
                        updateSymmetricFrequencyDomainData(fftOutput);
                        
                        // Inverse FFT
                        fft -> performRealOnlyInverseTransform(fftOutput);
                        
                        // OLA
                        float residualGainLinear = Decibels::decibelsToGain(gainBeam[beamIdx]->get())/commonGainLinear;
                        beamBuffer.addFrom(beamIdx, subBlockFirstIdx, fftOutput, FFT_SIZE, residualGainLinear);
                    }
                }
                
            }
            
        }
        
    }
    
    // Meter
    {
        float minVal,maxVal,maxAbsVal;
        beamMetersLock.enter();
        for (auto beamIdx = 0; beamIdx < NUM_BEAMS; ++beamIdx)
        {
#ifdef METERS_MODE_RMS
            beamMeters[beamIdx] = beamMeters[beamIdx]*(METERS_INERTIA) + beamBuffer.getRMSLevel(beamIdx, 0, blockNumSamples)*(1-METERS_INERTIA);
#else
            findMinAndMax(beamBuffer.getReadPointer(beamIdx), blockNumSamples, minVal, maxVal);
            maxAbsVal = jmax(abs(minVal),abs(maxVal));
            beamMeters[beamIdx] = beamMeters[beamIdx]*(METERS_INERTIA) + maxAbsVal*(1-METERS_INERTIA);
#endif
        }
        beamMetersLock.exit();
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
        xml->setAttribute(Identifier(stringStreamTag.str()), (double) *(gainBeam[beamIdx]));

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
                *(gainBeam[beamIdx]) = xmlState->getDoubleAttribute (Identifier(stringStreamTag.str()), 10.0);
                
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
    if(fftData.size() > BUFFER_UPPER_THRESHOLD)
        return (fftData.size() - BUFFER_UPPER_THRESHOLD);
        
    if(fftData.size() < BUFFER_LOWER_THRESHOLD)
        return (fftData.size() - BUFFER_LOWER_THRESHOLD);
        
    return 0;
}

std::vector<float*> JucebeamAudioProcessor::popFrontFFTdata()
{
    std::vector<float*> result;
    
    if(fftData.size() == 0)
        return result; // The buffer is empty
        
    if(fftData.size() == 1 && fftData.back().size() < getTotalNumInputChannels())
        return result; // The last element of the buffer is still being filled
    
    result = fftData.front();
    
    fftData.erase(fftData.begin());
    
    return result;
}

void JucebeamAudioProcessor::pushBackFFTdata(float* input)
{
    if(fftData.size() > 0 && fftData.back().size() < getTotalNumInputChannels()){
        // The last element of the buffer is still being filled
        fftData.back().push_back(input);
    }
    else{
        // The fft to be pushed is the first one of a new block
        fftData.emplace_back();
        fftData.back().push_back(input);
    }
}

