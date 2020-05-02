/*
 Meter components
 
 Authors:
 Luca Bondi (luca.bondi@polimi.it)
*/

#include "MeterComp.h"


void RoundLed::paint(Graphics &g) {

    Rectangle<float> area = getLocalBounds().toFloat();
    auto side = area.getHeight() > area.getWidth() ? area.getWidth() : area.getHeight();
    auto ctr = area.getCentre();
    area = Rectangle<float>(side, side);
    area.setCentre(ctr);

    g.setColour(colour);
    g.fillEllipse(area);
}

void RoundLed::resized() {

}


void MultiChannelLedBar::makeLayout() {
    removeAllChildren();
    leds.clear();
    if (callback == nullptr) {
        return;
    }
    callback->getMeterValues(values, meterId);
    auto num = values.size();
    for (auto ledIdx = 0; ledIdx < num; ++ledIdx) {
        leds.push_back(std::make_unique<RoundLed>());
        leds[ledIdx]->colour = Colours::grey;
        addAndMakeVisible(leds[ledIdx].get());
    }
    resized();
}

void MultiChannelLedBar::paint(Graphics &) {

}

void MultiChannelLedBar::resized() {

    if (callback == nullptr) {
        return;
    }
    callback->getMeterValues(values, meterId);
    auto num = values.size();
    Rectangle<int> area = getLocalBounds();

    int step = isHorizontal ? floor(area.getWidth() / num) : floor(area.getHeight() / num);
    int otherDim = isHorizontal ? area.getHeight() : area.getWidth();
    otherDim = jmin(otherDim, step - 1);

    const auto areaCtr = area.getCentre();

    // Re-center the area
    if (isHorizontal) {
        area.setWidth((int) (step * num));
        area.setHeight(otherDim);
    } else {
        area.setHeight((int) (step * num));
        area.setWidth(otherDim);
    }
    area.setCentre(areaCtr);

    for (auto ledIdx = 0; ledIdx < num; ++ledIdx) {
        Rectangle<int> ledArea = isHorizontal ? area.removeFromLeft(step) : area.removeFromTop(step);
        leds[ledIdx]->setBounds(ledArea);
    }

}

void MultiChannelLedBar::timerCallback() {

    if (callback == nullptr)
        return;

    callback->getMeterValues(values, meterId);

    if (values.size() != leds.size()) {
        makeLayout();
    }

    for (auto ledIdx = 0; ledIdx < leds.size(); ++ledIdx) {
        auto value = values.at(ledIdx);
        Colour col;
        if (value > Decibels::decibelsToGain(RED_LT)) {
            col = Colours::red;
        } else if (value > Decibels::decibelsToGain(YELLOW_LT)) {
            col = Colours::yellow;
        } else if (value > Decibels::decibelsToGain(GREEN_LT)) {
            col = Colours::lightgreen;
        } else {
            col = Colours::grey;
        }
        leds[ledIdx]->colour = col;
    }

    repaint();
}

void MultiChannelLedBar::setCallback(MeterDecay::Callback *cb, int metId) {
    callback = cb;
    meterId = metId;
    callback->getMeterValues(values, meterId);
    if (values.size() > 0) {
        makeLayout();
    }
}


SingleChannelLedBar::SingleChannelLedBar(size_t numLeds, bool isHorizontal) {
    jassert(numLeds > 4);

    this->isHorizontal = isHorizontal;

    const float ledStep = 3; //dB

    leds.clear();
    th.clear();
    for (auto ledIdx = 0; ledIdx < numLeds; ++ledIdx) {
        leds.push_back(std::make_unique<RoundLed>());

        auto ledThDb = ledIdx == (numLeds - 1) ? RED_LT : -((numLeds - 1 - ledIdx) * ledStep);
        th.push_back(ledThDb);
        leds[ledIdx]->colour = thToColour(ledThDb, false);

        addAndMakeVisible(leds[ledIdx].get());
    }
}

void SingleChannelLedBar::setCallback(MeterDecay::Callback *pr, int metId, int ch) {
    provider = pr;
    meterId = metId;
    channel = ch;
}

void SingleChannelLedBar::paint(Graphics &) {

}

void SingleChannelLedBar::resized() {

    Rectangle<float> area = getLocalBounds().toFloat();
    auto num = leds.size();
    float step = isHorizontal ? area.getWidth() / num : area.getHeight() / num;
    for (auto ledIdx = 0; ledIdx < num; ++ledIdx) {
        Rectangle<float> ledArea = isHorizontal ? area.removeFromLeft(step) : area.removeFromBottom(step);
        leds[ledIdx]->setBounds(ledArea.toNearestInt());
    }

}

void SingleChannelLedBar::timerCallback() {
    if (provider == nullptr)
        return;

    auto valueDb = Decibels::gainToDecibels(provider->getMeterValue(meterId, channel));
    for (auto ledIdx = 0; ledIdx < leds.size(); ++ledIdx)
        leds[ledIdx]->colour = thToColour(th[ledIdx], valueDb > th[ledIdx]);

    repaint();
}

Colour SingleChannelLedBar::thToColour(float th, bool active) {
    if (th >= RED_LT) {
        if (active)
            return Colours::red;
        else
            return Colours::darkred;
    } else if (th >= YELLOW_LT) {
        if (active)
            return Colours::yellow;
        else
            return Colours::darkgoldenrod;
    } else {
        if (active)
            return Colours::lightgreen;
        else
            return Colours::darkgreen;
    }
}
