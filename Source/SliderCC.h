/*
 ==============================================================================
 
 SliderCC.h
 Created: 22 Apr 2020 12:32:05pm
 Author:  Luca Bondi
 
 ==============================================================================
 */

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class MidiCCPopup
{
public:
    MidiCCPopup(Component& owner);
    ~MidiCCPopup();
    
    void setProcessorParamName(EbeamerAudioProcessor* proc, const String & param);
    
    void showPopupMenu();
    
private:
    EbeamerAudioProcessor* processor = nullptr;
    String paramName = "";
    Rectangle<int> popupArea;
    Component& owner;
    static void sliderMenuCallback (int result, MidiCCPopup* popup);
    
    bool isLearning() const;
};

//==============================================================================
class SliderCC    : public Slider, public MidiCCPopup{
public:
    SliderCC():MidiCCPopup(dynamic_cast<Slider&>(*this)){};
    ~SliderCC() override {};
    
    void mouseDown (const MouseEvent& e) override{
        if (e.mods.isPopupMenu()){
            MidiCCPopup::showPopupMenu();
        }else{
            Slider::mouseDown(e);
        }
    }
    
};

//==============================================================================
class TextButtonCC    : public TextButton, public MidiCCPopup{
public:
    TextButtonCC():MidiCCPopup(dynamic_cast<TextButton&>(*this)){};
    ~TextButtonCC()  override {};
    
    void mouseDown (const MouseEvent& e) override{
        if (e.mods.isPopupMenu()){
            MidiCCPopup::showPopupMenu();
        }else{
            TextButton::mouseDown(e);
        }
    }
    
};

//==============================================================================
class ToggleButtonCC    : public ToggleButton, public MidiCCPopup{
public:
    ToggleButtonCC():MidiCCPopup(dynamic_cast<ToggleButton&>(*this)){};
    ~ToggleButtonCC() override {};
    
    void mouseDown (const MouseEvent& e) override{
        if (e.mods.isPopupMenu()){
            MidiCCPopup::showPopupMenu();
        }else{
            ToggleButton::mouseDown(e);
        }
    }
    
};
