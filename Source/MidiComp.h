/*
 Base classes for GUI components used with MIDI CC
 
 Authors:
 Luca Bondi (luca.bondi@polimi.it)
*/

#pragma once
#define DB_MINUS_INF -100.0

#include "../JuceLibraryCode/JuceHeader.h"
#include "MidiCC.h"

//==============================================================================
class MidiCCPopup {
public:
    MidiCCPopup(Component &owner);

    ~MidiCCPopup(){};

    void setCallback(MidiCC::Callback *cb, const String &param);

    void showPopupMenu();

private:
    MidiCC::Callback *callback = nullptr;
    String paramName = "";
    Rectangle<int> popupArea;
    Component &owner;

    static void sliderMenuCallback(int result, MidiCCPopup *popup);

    bool isLearning() const;
};

//==============================================================================
class SliderCC : public Slider, public MidiCCPopup {
public:
    SliderCC() : MidiCCPopup(dynamic_cast<Slider &>(*this)) {};

    ~SliderCC() override {};

    void mouseDown(const MouseEvent &e) override {
        if (e.mods.isPopupMenu()) {
            MidiCCPopup::showPopupMenu();
        } else {
            Slider::mouseDown(e);
        }
    }

};

//==============================================================================
class TextButtonCC : public TextButton, public MidiCCPopup {
public:
    TextButtonCC() : MidiCCPopup(dynamic_cast<TextButton &>(*this)) {};

    ~TextButtonCC() override {};

    void mouseDown(const MouseEvent &e) override {

        if (e.mods.isPopupMenu()) {
            MidiCCPopup::showPopupMenu();
        } else {
            TextButton::mouseDown(e);
        }
    }
    
    void mouseUp(const MouseEvent &e) override {
        if (!e.mods.isPopupMenu()){
            TextButton::mouseUp(e);
        }
    }

};

//==============================================================================
class MuteButton : public TextButtonCC {
public:

    MuteButton(){
        setClickingTogglesState(true);
    };

};

//==============================================================================
class ToggleButtonCC : public ToggleButton, public MidiCCPopup {
public:
    ToggleButtonCC() : MidiCCPopup(dynamic_cast<ToggleButton &>(*this)) {};

    ~ToggleButtonCC() override {};

    void mouseDown(const MouseEvent &e) override {
        if (e.mods.isPopupMenu()) {
            MidiCCPopup::showPopupMenu();
        } else {
            ToggleButton::mouseDown(e);
        }
    }
    
    void mouseUp(const MouseEvent &e) override {
        if (!e.mods.isPopupMenu()){
            ToggleButton::mouseUp(e);
        }
    }
};

//==============================================================================
class DecibelSlider : public SliderCC {
public:

    DecibelSlider() {};

    double getValueFromText(const String &text) override {

        auto decibelText = text.upToFirstOccurrenceOf("dB", false, false).trim();

        return decibelText.equalsIgnoreCase("-INF") ? DB_MINUS_INF
                                                    : decibelText.getDoubleValue();  // [2]
    }

    String getTextFromValue(double value) override {
        return Decibels::toString(value, 1, DB_MINUS_INF);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DecibelSlider)
};

//==============================================================================

class FrequencySlider : public SliderCC {
public:

    FrequencySlider() {};

    double getValueFromText(const String &text) override {

        auto hzText = text.upToFirstOccurrenceOf("Hz", false, false).trim();
        auto hzVal = roundToInt(hzText.getDoubleValue());
        return hzVal;
    }

    String getTextFromValue(double value) override {
        std::ostringstream valueString;
        valueString << roundToInt(value) << " Hz";
        return valueString.str();
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FrequencySlider)
};

//==============================================================================

class PanSlider : public SliderCC {
public:

    PanSlider() {};

    double getValueFromText(const String &text) override {

        if (text.startsWithIgnoreCase("C") || text.startsWithIgnoreCase("0")) {
            return 0;
        }

        float sign = 1;
        if (text.endsWithIgnoreCase("L")) {
            sign = -1;
        }

        String valueText = text.upToFirstOccurrenceOf("%", false, false).trim();
        double val = valueText.getDoubleValue() / 100.;
        return sign * val;
    }

    String getTextFromValue(double value) override {
        jassert(value >= -1 && value <= 1);
        std::ostringstream valueString;
        if (value == 0) {
            valueString << "C";
        } else if (value < 0) {
            valueString << roundToInt(abs(value) * 100) << " % L";
        } else {
            valueString << roundToInt((value) * 100) << " % R";
        }

        return valueString.str();
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PanSlider)
};


