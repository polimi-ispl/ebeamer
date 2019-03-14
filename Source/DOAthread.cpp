#include "DOAthread.h"

//==============================================================================

DOAthread::DOAthread(JucebeamAudioProcessor& p, JucebeamAudioProcessorEditor& e)
        : processor(p), editor(e), Thread("Direction of arrival thread")
{
    startThread(3);
    
    const GenericScopedLock<SpinLock> scopedLock (energyLock);
    
    for(float i = 0; i <= 1; i+=1/25){
        energy.push_back(i);
    }
}

DOAthread::~DOAthread()
{
    stopThread(2000);
}

//==============================================================================

void DOAthread::run()
{
    std::vector<float> temp;
    
    while(!threadShouldExit())
    {
        wait(500);
        
        temp = energy;
        temp.push_back(temp.front());
        temp.erase(temp.begin());
        
        const GenericScopedLock<SpinLock> scopedLock (energyLock);
        
        energy = temp;
    }
}

//==============================================================================

std::vector<float> DOAthread::getEnergy()
{
    const GenericScopedLock<SpinLock> scopedLock (energyLock);
    
    return energy;
}
