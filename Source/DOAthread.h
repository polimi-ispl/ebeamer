#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================

// These need to be declared in order to have references to them in DOAthread;
// they'll be defined later because they contain instances of DOAthread.
class JucebeamAudioProcessor;
class JucebeamAudioProcessorEditor;

//==============================================================================

class DOAthread     : public Thread
{
public:

    DOAthread(JucebeamAudioProcessor&, JucebeamAudioProcessorEditor&);
    ~DOAthread();
    
    void run() override;

//==============================================================================
    
private:

    JucebeamAudioProcessor& processor;
    JucebeamAudioProcessorEditor& editor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DOAthread);
};