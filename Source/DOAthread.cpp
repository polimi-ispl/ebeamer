#include "DOAthread.h"

//==============================================================================

DOAthread::DOAthread(JucebeamAudioProcessor& p)
        : Thread("Direction of arrival thread"),  processor(p)
{
    startThread(3);
    
    const GenericScopedLock<SpinLock> scopedLock(energyLock);
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
    int interval = 50;
    
    while(!threadShouldExit())
    {
        const GenericScopedLock<SpinLock> scopedFFTlock(processor.fftLock);
        
        // Check buffer size to assess performance.
        int status = processor.bufferStatus();
        
        if(status < 0){
            // DOAthread is too fast.
            // Increase considered directions, or frequencies, or ...
            interval += 5;
            // continue;
        }
        
        if(status > 0){
            // DOAthread is too slow.
            // Decrease considered directions, or frequencies, or ...
            if(interval > 5)
                interval -= 5;
        }
        
        if(status == 0){
            // DOAthread is on time.
        }
        
        // Retrieve fft data from processor.
        
        fftData = processor.popFrontFFTdata();
        
        const GenericScopedUnlock<SpinLock> scopedFFTunlock(processor.fftLock);
        
        wait(interval);
        
        // Compute energy, stored in temp.
        
        // This displays the energy as a ramp,
        // higher on the right if the buffer is large,
        // higher on the left if the buffer is almost empty.
        // DOAthread starts later than the processor,
        // so it has to catch up in the first few moments.
        temp.clear();
        for(int i = 0; i < INITIAL_CONSIDERED_DIRECTIONS; i++){
            float j = (float)i / INITIAL_CONSIDERED_DIRECTIONS;
            j = 0.25 + (status / 10) * (j - 0.25);
            temp.push_back(j);
        }
        
        const GenericScopedLock<SpinLock> scopedEnergyLock(energyLock);
        
        energy = temp;
    }
}

//==============================================================================

std::vector<float> DOAthread::getEnergy()
{
    const GenericScopedLock<SpinLock> scopedLock(energyLock);
    
    return energy;
}
