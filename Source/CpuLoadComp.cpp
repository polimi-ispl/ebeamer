/*
 CPU load component
 
 Authors:
 Luca Bondi (luca.bondi@polimi.it)
*/

#include <JuceHeader.h>
#include "CpuLoadComp.h"

//==============================================================================
CpuLoadComp::CpuLoadComp() {

    text.setText("0 %", NotificationType::dontSendNotification);
    label.setText("CPU", NotificationType::dontSendNotification);
    label.setJustificationType(Justification::left);
    label.attachToComponent(&text, true);
    addAndMakeVisible(text);
}

CpuLoadComp::~CpuLoadComp() {
}

void CpuLoadComp::paint(Graphics &g) {
}

void CpuLoadComp::setSource(Callback *cb) {
    callback = cb;
}

void CpuLoadComp::resized() {
    auto area = getLocalBounds();
    area.removeFromLeft(labelWidth);
    text.setBounds(area);
}

void CpuLoadComp::timerCallback() {
    if (callback == nullptr)
        return;
    text.setText(String(int(callback->getCpuLoad() * 100)) + "%", NotificationType::dontSendNotification);
}
