#include "vDOAthread.h"

//==============================================================================

vDOAthread::vDOAthread(JucebeamAudioProcessor& p)
        : Thread("DOA"),  processor(p)
{
    
    directionIdxs.clear();
    for (auto idx = 0; idx < processor.getMimoProcessor()->getNumSteeringFir(); ++idx)
    {
        directionIdxs.push_back(idx);
    }
    
    energy.clear();
    energy.resize(directionIdxs.size());
    
    fftOutput = AudioBuffer<float>(1,2*processor.getMimoProcessor()->getFftSize());
    directionalSignal = AudioBuffer<float>(1,processor.getMimoProcessor()->getFftSize());
    
    fft = std::make_unique<dsp::FFT>(ceil(log2(processor.getMimoProcessor()->getFftSize())));
    
    // Initialize HPF and LPF
    iirCoeffHPF = IIRCoefficients::makeHighPass(processor.getSampleRate(), 500);
    iirHPFfilter = std::make_unique<IIRFilter>();
    iirHPFfilter->setCoefficients(iirCoeffHPF);
    
    iirCoeffLPF = IIRCoefficients::makeLowPass(processor.getSampleRate(), 5000);
    iirLPFfilter = std::make_unique<IIRFilter>();
    iirLPFfilter->setCoefficients(iirCoeffLPF);
    
}

vDOAthread::~vDOAthread()
{
    stopThread(100);
}

//==============================================================================

void vDOAthread::run()
{
    ScopedNoDenormals noDenormals;
    
    std::vector<float> newEnergy;
    std::vector<float> prevEnergy;
    
    while(!threadShouldExit())
    {
     
        directionalSignal.setSize(1, processor.getMimoProcessor()->getFftSize());
        
        while (!threadShouldExit() && newEnergyAvailable)
        {
            // Wait to produce new energy estimate till the GUI consumes it
            sleep (10);
        }

        while ((!threadShouldExit()) && (! processor.newFftInputDataAvailable))
        {
            // Wait until new data awailable
            sleep (10);
        }

        {
            GenericScopedLock<SpinLock> lock(processor.fftInputLock);
            fftInput.makeCopyOf(processor.fftInput);
            processor.newFftInputDataAvailable = false;
        }
        
        int fftSize = processor.getMimoProcessor()->getFftSize();
        if (fftInput.getNumSamples() != 2*fft->getSize()){
            fft = std::make_unique<dsp::FFT>(ceil(log2(fftSize)));
            fftOutput.setSize(1, 2*fftSize);
        }
        
        prevEnergy = energy;
        newEnergy.clear();
        newEnergy.resize(directionIdxs.size());
        
        for (auto dirIdx = 0; dirIdx < directionIdxs.size(); ++dirIdx)
        {
            
            directionalSignal.clear();
            
            int steeringIdx = directionIdxs[dirIdx];
            
            for (auto inChannel = 0; inChannel < fftInput.getNumChannels(); ++inChannel)
            {
                
                fftOutput.clear();
                vFIR::convolutionProcessingAndAccumulate(fftInput.getReadPointer(inChannel),processor.getMimoProcessor()->getFirSteeringFFT(steeringIdx,inChannel),fftOutput.getWritePointer(0),fftSize);
                 vFIR::updateSymmetricFrequencyDomainData(fftOutput.getWritePointer(0),fftSize);
                
                fft -> performRealOnlyInverseTransform(fftOutput.getWritePointer(0));
                
                
                directionalSignal.addFrom(0, 0, fftOutput, 0, 0, fftSize);
                
            }
            
            iirHPFfilter->processSamples(directionalSignal.getWritePointer(0), directionalSignal.getNumSamples());
            iirLPFfilter->processSamples(directionalSignal.getWritePointer(0), directionalSignal.getNumSamples());
            
            auto range = FloatVectorOperations::findMinAndMax(directionalSignal.getReadPointer(0), directionalSignal.getNumSamples());
            auto maxAbs = jmax(abs(range.getStart()),abs(range.getEnd()));
            auto maxAbsDb = Decibels::gainToDecibels(maxAbs);
            
            newEnergy[dirIdx] = ((1-inertia) * (maxAbsDb + gain)) + (inertia * prevEnergy[dirIdx]);
            
        }
        
        // Automatic gain
        auto rangeEnergy = FloatVectorOperations::findMinAndMax(newEnergy.data(), (int)newEnergy.size());
        if (gain > minGain &&
            rangeEnergy.getEnd() > 0){
            gain-=2;
        }
        else if (gain < maxGain &&
             rangeEnergy.getEnd() < -18){
            gain+=2;
        }
        else if (gain < maxGain &&
            rangeEnergy.getEnd() < -9 &&
            rangeEnergy.getLength() > 15)
        {
            gain+=0.5;
        }
        else if (gain > minGain &&
             rangeEnergy.getStart() > -9){
            gain-=0.5;
        }
        else

        // Make new energy available
        {
            GenericScopedLock<SpinLock> lock(energyLock);
            energy = newEnergy;
            newEnergyAvailable = true;
        }
        
    }
}

//==============================================================================
