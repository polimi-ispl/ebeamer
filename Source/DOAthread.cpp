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
    ScopedNoDenormals noDenormals;
    
    std::vector<float*> fftData;
    std::vector<float> tempEnergy;
    std::vector<float> prevEnergy;
    
    int interval = 50;
    
    float fftOutput[2*FFT_SIZE];
    
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
        
        energyLock.enter();
        prevEnergy = energy;
        energyLock.exit();
        
        tempEnergy.clear();
        tempEnergy.resize(INITIAL_CONSIDERED_DIRECTIONS);
        
        for(int inChannel = 0; inChannel < fftData.size(); ++inChannel){
            for(int beamIdx = 0; beamIdx < INITIAL_CONSIDERED_DIRECTIONS; ++beamIdx){
                
                int steeringIdx = round(beamIdx / (INITIAL_CONSIDERED_DIRECTIONS - 1));
                int beamWidthIdx = 0;
                
                // FIR processing (includes reverse FFT)
                processor.firConvolve(fftData.at(inChannel), fftOutput, inChannel, beamWidthIdx, steeringIdx);
                
                tempEnergy.at(beamIdx) = prevEnergy.at(beamIdx);
                
                for(int t = 0; t < FFT_SIZE; t++)
                    if(fftOutput[t] > tempEnergy.at(beamIdx) * EXP_DECAY_RATE)
                        tempEnergy.at(beamIdx) = fftOutput[t];
            }
        }
        
        energyLock.enter();
        energy = tempEnergy;
        energyLock.exit();
    }
}

//==============================================================================
