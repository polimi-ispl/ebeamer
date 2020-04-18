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

    loadText.setFont(textHeight);
    loadText.setText(String(int(processor.getAverageLoad()*100))+"%");
    loadText.setReadOnly(true);
    loadTextLabel.setFont(textHeight);
    loadTextLabel.setText("CPU load", NotificationType::dontSendNotification);
    loadTextLabel.setJustificationType(Justification::left);
    loadTextLabel.attachToComponent(&loadText, true);
    addAndMakeVisible(loadText);
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
    loadText.setBounds(area);
}

void CpuLoadComp::timerCallback(){
    loadText.setText(String(int(processor.getAverageLoad()*100))+"%");
}
