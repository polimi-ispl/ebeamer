#include "DOAthread.h"

//==============================================================================

DOAthread::DOAthread(JucebeamAudioProcessor& p, JucebeamAudioProcessorEditor& e)
        : processor(p), editor(e), Thread("Direction of arrival thread")
{
    startThread(3);
}

DOAthread::~DOAthread()
{
    stopThread(2000);
}

void DOAthread::run()
{
    while(!threadShouldExit())
    {
    }
}

//==============================================================================