/*
 eBeamer Plugin Processor GUI
 
 Authors:
 Luca Bondi (luca.bondi@polimi.it)
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"



EBeamerAudioProcessorEditor::EBeamerAudioProcessorEditor(EbeamerAudioProcessor &p, ValueTree vt)
: AudioProcessorEditor(&p)
{    
    //==============================================================================
    setResizable (true, true);
    setResizeLimits(MIN_WIDTH, MIN_HEIGHT, MAX_WIDTH, MAX_HEIGHT);
    
    setSize(GUI_WIDTH, GUI_HEIGHT);
    
    //==============================================================================
    /* Init GUI */
    
    gui.initServer("ISPL and Eventide",JucePlugin_Name,JucePlugin_VersionString,vt,this);
    gui.enableMidi(&p);
    addAndMakeVisible(gui);
    
}

EBeamerAudioProcessorEditor::~EBeamerAudioProcessorEditor() {
}

//==============================================================================

void EBeamerAudioProcessorEditor::paint(Graphics &g) {
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
    
}

void EBeamerAudioProcessorEditor::resized() {
    
    gui.setBounds(getLocalBounds());
    
}
