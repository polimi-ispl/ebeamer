/*
  ==============================================================================

    CpuLoadComp.cpp
    Created: 18 Apr 2020 4:23:20pm
    Author:  Luca Bondi

  ==============================================================================
*/

#include <JuceHeader.h>
#include "CpuLoadComp.h"

//==============================================================================
CpuLoadComp::CpuLoadComp(const EbeamerAudioProcessor &p):processor(p)
{

    text.setFont(textHeight);
    text.setText(String(int(processor.getAverageLoad()*100))+"%");
    text.setReadOnly(true);
    label.setFont(textHeight);
    label.setText("CPU load", NotificationType::dontSendNotification);
    label.setJustificationType(Justification::left);
    label.attachToComponent(&text, true);
    addAndMakeVisible(text);
}

CpuLoadComp::~CpuLoadComp()
{
}

void CpuLoadComp::paint (Graphics& g)
{
}

void CpuLoadComp::resized()
{
    auto area = getLocalBounds();
    area.removeFromLeft(labelWidth);
    text.setBounds(area);
}

void CpuLoadComp::timerCallback(){
    text.setText(String(int(processor.getAverageLoad()*100))+"%");
}
