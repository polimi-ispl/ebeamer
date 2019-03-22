#include "DOAthread.h"

//==============================================================================

DOAthread::DOAthread(JucebeamAudioProcessor& p)
        : Thread("Direction of arrival thread"),  processor(p)
{
    startThread(3);
    
    energy.clear();
    energy.resize(INITIAL_CONSIDERED_DIRECTIONS);
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
    
    float fftOutput[2*FFT_SIZE];
    
    while(!threadShouldExit())
    {
        const GenericScopedLock<SpinLock> scopedFFTlock(processor.fftLock);
        
        // Check buffer size to assess performance.
        int status = processor.bufferStatus();
        
        if(status < 0){
            // DOAthread is too fast.
            // Increase considered directions, or frequencies, or ...
            wait(50);
            continue;
        }
        
        if(status > 0){
            // DOAthread is too slow.
            // Decrease considered directions, or frequencies, or ...
        }
        
        if(status == 0){
            // DOAthread is on time.
        }
        
        // Retrieve fft data from processor.
        
        fftData = processor.popFrontFFTdata();
        
        const GenericScopedUnlock<SpinLock> scopedFFTunlock(processor.fftLock);
        
        // Compute energy, stored in temp.
        
        energyLock.enter();
        prevEnergy = energy;
        energyLock.exit();
        
        tempEnergy.clear();
        tempEnergy.resize(INITIAL_CONSIDERED_DIRECTIONS);
        
        for(int inChannel = 0; inChannel < fftData.size(); ++inChannel){
            for(int beamIdx = 0; beamIdx < INITIAL_CONSIDERED_DIRECTIONS; ++beamIdx){
                
                // FIR processing (includes reverse FFT)
                processor.firConvolve(
                                        fftData.at(inChannel),
                                        fftOutput,
                                        inChannel,
                                        (float) beamIdx / (INITIAL_CONSIDERED_DIRECTIONS - 1),
                                        0
                                     );
                
                tempEnergy.at(beamIdx) = prevEnergy.at(beamIdx);
                
                for(int t = 0; t < FFT_SIZE; t++)
                    if(25 * fftOutput[t] > tempEnergy.at(beamIdx) * EXP_DECAY_RATE)
                        tempEnergy.at(beamIdx) = 25 * fftOutput[t];
                    else
                        tempEnergy.at(beamIdx) = tempEnergy.at(beamIdx) * EXP_DECAY_RATE;
                
                /*       
                float minVal,
                      maxVal,
                      maxAbsVal;
                
                    findMinAndMax(fftOutput, FFT_SIZE, minVal, maxVal);
                    maxAbsVal = jmax(abs(minVal), abs(maxVal));
                    tempEnergy.at(beamIdx) = prevEnergy.at(beamIdx)
                                            *
                                            (EXP_DECAY_RATE)
                                            +
                                            maxAbsVal
                                            *
                                            (1 - EXP_DECAY_RATE);
                                            */
            }
        }
        
        energyLock.enter();
        energy = tempEnergy;
        energyLock.exit();
    }
}

//==============================================================================
