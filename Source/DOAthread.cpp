#include "DOAthread.h"

//==============================================================================

DOAthread::DOAthread(JucebeamAudioProcessor& p)
        : Thread("DOA"),  processor(p)
{
    
    directionIdxs.clear();
    for (auto idx = 0; idx < processor.getMimoProcessor()->getNumSteeringFir(); ++idx)
    {
        directionIdxs.push_back(idx);
    }
    
    energy.clear();
    energy.resize(directionIdxs.size());
        
    // Initialize HPF and LPF
    iirCoeffHPF = IIRCoefficients::makeHighPass(processor.getSampleRate(), 500);
    iirHPFfilter = std::make_unique<IIRFilter>();
    iirHPFfilter->setCoefficients(iirCoeffHPF);
    
    iirCoeffLPF = IIRCoefficients::makeLowPass(processor.getSampleRate(), 5000);
    iirLPFfilter = std::make_unique<IIRFilter>();
    iirLPFfilter->setCoefficients(iirCoeffLPF);
    
}

DOAthread::~DOAthread()
{
    stopThread(100);
}

//==============================================================================

void DOAthread::run()
{
    ScopedNoDenormals noDenormals;
    
    std::vector<float> newEnergy;
    std::vector<float> prevEnergy;
    
    while(!threadShouldExit())
    {
     
        directionalSignal.setSize(1, processor.getMimoProcessor()->fft->getSize());
        
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
            inputsFFT = processor.inputsFFT;
            processor.newFftInputDataAvailable = false;
        }

        prevEnergy = energy;
        newEnergy.clear();
        newEnergy.resize(directionIdxs.size());
        
        for (auto dirIdx = 0; dirIdx < directionIdxs.size(); ++dirIdx)
        {
            processor.getMimoProcessor()->newBlock(directionalSignal);
            int steeringIdx = directionIdxs[dirIdx];
            for (auto inChannel = 0; inChannel < inputsFFT.getNumChannels(); ++inChannel)
            {
                processor.getMimoProcessor()->processBlock(inputsFFT,inChannel,directionalSignal,0,steeringIdx,-1);
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
