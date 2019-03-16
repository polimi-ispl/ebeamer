/*
  ==============================================================================

    AudioComponents.cpp
    Created: 16 Mar 2019 11:44:28am
    Author:  Luca Bondi

  ==============================================================================
*/

#include "AudioComponents.h"

void LedComponent::paint(Graphics& g){
    
    Rectangle<float> area = getLocalBounds().toFloat();
    auto side = area.getHeight() > area.getWidth() ? area.getWidth() : area.getHeight();
    auto ctr = area.getCentre();
    area = Rectangle<float>(side,side);
    area.setCentre(ctr);
    
    g.setColour(colour);
    g.fillEllipse(area);
}

void LedComponent::resized(){
    
}


LedBarComponent::LedBarComponent(int num, bool isHorizontal){
    jassert(num > 0);
    this->num = num;
    this->isHorizontal = isHorizontal;
    
    leds.clear();
    for (auto ledIdx = 0; ledIdx < num; ++ledIdx)
    {
        leds.push_back(std::make_unique<LedComponent>());
        leds[ledIdx]->colour = Colours::grey;
        addAndMakeVisible(leds[ledIdx].get());
    }
}

void LedBarComponent::paint(Graphics& g){

}

void LedBarComponent::resized(){
    
    Rectangle<float> area = getLocalBounds().toFloat();
    float step = isHorizontal ? area.getWidth()/num : area.getHeight()/num;
    for (auto ledIdx = 0; ledIdx < num; ++ledIdx)
    {
        Rectangle<float> ledArea = isHorizontal ? area.removeFromLeft(step) : area.removeFromTop(step);
        leds[ledIdx]->setBounds(ledArea.toNearestInt());
    }
    
}
