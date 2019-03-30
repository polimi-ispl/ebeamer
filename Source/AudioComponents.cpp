/*
 ==============================================================================
 
 AudioComponents.cpp
 Created: 16 Mar 2019 11:44:28am
 Author:  Luca Bondi
 
 ==============================================================================
 */

#include "AudioComponents.h"



void RoundLed::paint(Graphics& g){
    
    Rectangle<float> area = getLocalBounds().toFloat();
    auto side = area.getHeight() > area.getWidth() ? area.getWidth() : area.getHeight();
    auto ctr = area.getCentre();
    area = Rectangle<float>(side,side);
    area.setCentre(ctr);
    
    g.setColour(colour);
    g.fillEllipse(area);
}

void RoundLed::resized(){
    
}


void MultiChannelLedBar::makeLayout()
{
    removeAllChildren();
    leds.clear();
    for (auto ledIdx = 0; ledIdx < num; ++ledIdx)
    {
        leds.push_back(std::make_unique<RoundLed>());
        leds[ledIdx]->colour = Colours::grey;
        addAndMakeVisible(leds[ledIdx].get());
    }
    resized();
}

void MultiChannelLedBar::paint(Graphics& g){
    
}

void MultiChannelLedBar::resized(){
    
    Rectangle<float> area = getLocalBounds().toFloat();
    float step = isHorizontal ? area.getWidth()/num : area.getHeight()/num;
    for (auto ledIdx = 0; ledIdx < num; ++ledIdx)
    {
        Rectangle<float> ledArea = isHorizontal ? area.removeFromLeft(step) : area.removeFromTop(step);
        leds[ledIdx]->setBounds(ledArea.toNearestInt());
    }
    
}

void MultiChannelLedBar::timerCallback()
{
    
    if (source == nullptr)
        return;
    
    lock->enter();
    std::vector<float> values = *source;
    lock->exit();
    
    for (auto ledIdx = 0; ledIdx < leds.size(); ++ledIdx)
    {
        auto value = values.at(ledIdx);
        Colour col;
        if (value > Decibels::decibelsToGain(RED_LT))
        {
            col = Colours::red;
        }else if(value > Decibels::decibelsToGain(YELLOW_LT))
        {
            col = Colours::yellow;
        }
        else if (value > Decibels::decibelsToGain(GREEN_LT))
        {
            col = Colours::lightgreen;
        }
        else
        {
            col = Colours::grey;
        }
        leds[ledIdx]->colour = col;
    }
    
    repaint();
}

void MultiChannelLedBar::setSource(const std::vector<float> &source,SpinLock &lock)
{
    num = source.size();
    makeLayout();
    this->source = &source;
    this->lock = &lock;
}


SingleChannelLedBar::SingleChannelLedBar(size_t numLeds, bool isHorizontal){
    jassert(numLeds > 4);
    
    this->num = numLeds;
    this->isHorizontal = isHorizontal;
    
    const float ledStep = 3; //dB
    
    leds.clear();
    th.clear();
    for (auto ledIdx = 0; ledIdx < num; ++ledIdx)
    {
        leds.push_back(std::make_unique<RoundLed>());
        
        auto ledThDb = ledIdx == (num-1) ? RED_LT : -((num - 1 - ledIdx) *ledStep);
        th.push_back(ledThDb);
        leds[ledIdx]->colour = thToColour(ledThDb,false);
        
        addAndMakeVisible(leds[ledIdx].get());
    }
}

void SingleChannelLedBar::setSource(const std::vector<float> &source,int ch, SpinLock &lock){
    this->source = &source;
    this->ch = ch;
    this->lock = &lock;
}

void SingleChannelLedBar::paint(Graphics& g){
    
}

void SingleChannelLedBar::resized(){
    
    Rectangle<float> area = getLocalBounds().toFloat();
    float step = isHorizontal ? area.getWidth()/num : area.getHeight()/num;
    for (auto ledIdx = 0; ledIdx < num; ++ledIdx)
    {
        Rectangle<float> ledArea = isHorizontal ? area.removeFromLeft(step) : area.removeFromBottom(step);
        leds[ledIdx]->setBounds(ledArea.toNearestInt());
    }
    
}

void SingleChannelLedBar::timerCallback()
{
    lock->enter();
    auto valueDb = Decibels::gainToDecibels(source->at(ch));
    lock->exit();
    for (auto ledIdx = 0; ledIdx < leds.size(); ++ledIdx)
            leds[ledIdx]->colour = thToColour(th[ledIdx], valueDb > th[ledIdx]);
    
    repaint();
}

Colour SingleChannelLedBar::thToColour(float th, bool active)
{
    if (th >= RED_LT)
    {
        if (active)
            return Colours::red;
        else
            return Colours::darkred;
    }
    else if (th >= YELLOW_LT)
    {
        if (active)
            return Colours::yellow;
        else
            return Colours::darkgoldenrod;
    }
    else
    {
        if (active)
            return Colours::lightgreen;
        else
            return Colours::darkgreen;
    }
}
