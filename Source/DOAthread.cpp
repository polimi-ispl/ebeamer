#include "DOAthread.h"

//==============================================================================

DOAthread::DOAthread(JucebeamAudioProcessor& p)
        : processor(p), Thread("Direction of arrival thread")
{
    startThread(3);
    
    const GenericScopedLock<SpinLock> scopedLock(energyLock);
    
    for(int i = 0; i < INITIAL_CONSIDERED_DIRECTIONS; i++){
        energy.push_back((float)i/INITIAL_CONSIDERED_DIRECTIONS);
    }
}

DOAthread::~DOAthread()
{
    stopThread(2000);
}

//==============================================================================

void DOAthread::run()
{
    std::vector<float*> fftData;
    std::vector<float> temp;
    
    while(!threadShouldExit())
    {
        wait(500);
        
        // Check buffer size to assess performance
        
        if(processor.isBufferGrowing()){
            // DOAthread is too slow
            // Decrease considered directions, or frequencies, or ...
        }
        
        // Retrieve fft data from processor
        
        fftData = processor.popFrontFFTdata();
        
        if(fftData.size() == 0){
            // DOAthread is too fast
            wait(50);
            // Increase considered directions, or frequencies, or ...
            continue;
        }
        
        // Compute energy, stored in temp
        
        temp = energy;
        temp.push_back(temp.front());
        temp.erase(temp.begin());
        
        const GenericScopedLock<SpinLock> scopedLock(energyLock);
        
        energy = temp;
    }
}

//==============================================================================

std::vector<float> DOAthread::getEnergy()
{
    const GenericScopedLock<SpinLock> scopedLock(energyLock);
    
    return energy;
}
