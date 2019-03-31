#include "DOAthread.h"

//==============================================================================

DOAthread::DOAthread(JucebeamAudioProcessor& p)
        : Thread("DOA"),  processor(p)
{
    
    directionIdxs.clear();
    for (auto idx = 0; idx < processor.firSteeringFFT.size(); ++idx)
    {
        directionIdxs.push_back(idx);
    }
    
    energy.clear();
    energy.resize(directionIdxs.size());
    
    fftOutput = AudioBuffer<float>(1,2*processor.getFftSize());
    directionalSignal = AudioBuffer<float>(1,processor.getFftSize());
    
    fft = std::make_unique<dsp::FFT>(ceil(log2(processor.getFftSize())));
    
}

DOAthread::~DOAthread()
{
    stopThread(1000);
}

//==============================================================================

void DOAthread::run()
{
    ScopedNoDenormals noDenormals;
    
    std::vector<float> frameEnergy;
    std::vector<float> prevEnergy;
    
    while(!threadShouldExit())
    {
     
        directionalSignal.setSize(1, processor.getFftSize());
        
        while (newEnergyAvailable)
        {
            // Wait to produce new energy estimate till the GUI consumes it
            sleep (10);
        }

        while (! processor.newFftInputDataAvailable)
        {
            // Wait until new data awailable
            sleep (10);
        }

        {
            GenericScopedLock<SpinLock> lock(processor.fftInputLock);
            fftInput.makeCopyOf(processor.fftInput);
            processor.newFftInputDataAvailable = false;
        }
        
        if (fftInput.getNumSamples() != 2*fft->getSize()){
            fft = std::make_unique<dsp::FFT>(ceil(log2(processor.getFftSize())));
            fftOutput.setSize(1, 2*processor.getFftSize());
        }
        
        prevEnergy = energy;
        frameEnergy.clear();
        frameEnergy.resize(directionIdxs.size());
        
        for (auto dirIdx = 0; dirIdx < directionIdxs.size(); ++dirIdx)
        {
            
            directionalSignal.clear();
            
            int steeringIdx = directionIdxs[dirIdx];
            
            for (auto inChannel = 0; inChannel < fftInput.getNumChannels(); ++inChannel)
            {
                
                fftOutput.clear();
                 processor.convolutionProcessingAndAccumulate(fftInput.getReadPointer(inChannel),processor.firSteeringFFT[steeringIdx][inChannel].data(),fftOutput.getWritePointer(0),processor.getFftSize());
                 processor.updateSymmetricFrequencyDomainData(fftOutput.getWritePointer(0),processor.getFftSize());
                
                fft -> performRealOnlyInverseTransform(fftOutput.getWritePointer(0));
                
                directionalSignal.addFrom(0, 0, fftOutput, 0, 0, processor.getFftSize());
                
            }
            
            frameEnergy[dirIdx] = directionalSignal.getRMSLevel(0, 0, directionalSignal.getNumSamples());
            
        }

        {
            GenericScopedLock<SpinLock> lock(energyLock);
            energy = static_cast<const std::vector<float>>(frameEnergy);
            newEnergyAvailable = true;
        }
        
    }
}

//==============================================================================
