#include "DOAthread.h"

//==============================================================================

DOAthread::DOAthread(EbeamerAudioProcessor& p)
        : Thread("DOA"),  processor(p){
    energy.resize(EbeamerAudioProcessor::numDoas,-100);
}

DOAthread::~DOAthread(){
    stopThread(100);
}

//==============================================================================

void DOAthread::run(){
    ScopedNoDenormals noDenormals;
    
    std::vector<float> newEnergy(EbeamerAudioProcessor::numDoas);
    
    while(!threadShouldExit()){
     
        while (!threadShouldExit() && newEnergyAvailable){
            // Wait to produce new energy estimate till the GUI consumes it
            sleep (10);
        }
        
        processor.getBeamformer()->getDoaEnergy(newEnergy);
        for (auto dirIdx = 0; dirIdx < newEnergy.size(); ++dirIdx){
            newEnergy[dirIdx] = ((1-inertia) * (newEnergy[dirIdx] + gain)) + (inertia * energy[dirIdx]);
        }
        
        // Automatic gain
        auto rangeEnergy = FloatVectorOperations::findMinAndMax(newEnergy.data(), (int)newEnergy.size());
        if (gain > minGain && rangeEnergy.getEnd() > 0){
            gain-=2;
        }
        else if (gain < maxGain && rangeEnergy.getEnd() < -18){
            gain+=2;
        }
        else if (gain < maxGain && rangeEnergy.getEnd() < -9 && rangeEnergy.getLength() > 15)
        {
            gain+=0.5;
        }
        else if (gain > minGain && rangeEnergy.getStart() > -9){
            gain-=0.5;
        }
        
        {
            // Make new energy available
            GenericScopedLock<SpinLock> lock(energyLock);
            energy = newEnergy;
            newEnergyAvailable = true;
        }
        
    }
}

//==============================================================================
