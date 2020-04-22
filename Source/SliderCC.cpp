/*
  ==============================================================================

    SliderCC.cpp
    Created: 22 Apr 2020 12:32:05pm
    Author:  Luca Bondi

  ==============================================================================
*/

#include <JuceHeader.h>
#include "SliderCC.h"

//==============================================================================

MidiCCPopup::MidiCCPopup(Component& owner_):owner(owner_){
    
}

MidiCCPopup::~MidiCCPopup(){
    
}

void MidiCCPopup::setProcessorParamName(EbeamerAudioProcessor* proc, const String & param){
    processor = proc;
    paramName = param;
}

bool MidiCCPopup::isLearning() const{
    return paramName == processor->getCCLearning();
}

void MidiCCPopup::showPopupMenu()
{
    PopupMenu m;
    m.setLookAndFeel (&owner.getLookAndFeel());
    if (isLearning()){
        m.addItem(1,"Stop learning CC",true,false);
    }else{
        m.addItem(1,"Learn CC",true,false);
    }
    
    auto mapping = processor->getParamToCCMapping();
    if (mapping.count(paramName) > 0){
        m.addItem(2,"Forget CC ",true,false);
        m.addItem(3,"Chan: " + String(mapping[paramName].channel) + " - CC: " + String(mapping[paramName].number),false,false);
    }

    if (popupArea.getX() == 0){
        popupArea = PopupMenu::Options().getTargetScreenArea();
    }
    m.showAt(popupArea,0,0,0,0,ModalCallbackFunction::create (sliderMenuCallback, this));
    
}

void MidiCCPopup::sliderMenuCallback (int result, MidiCCPopup* popup)
{
    if (popup != nullptr)
    {
        switch (result)
        {
            case 1:
                if (!popup->isLearning()){
                    popup->processor->removeCCParamMapping(popup->paramName);
                    popup->processor->startCCLearning(popup->paramName);
                    popup->showPopupMenu();
                }else{
                    popup->processor->stopCCLearning();
                    popup->popupArea.setBounds(0,0,0,0);
                }
                break;
            case 2:
                popup->processor->removeCCParamMapping(popup->paramName);
                popup->popupArea.setBounds(0,0,0,0);
                break;
            default:  break;
        }
    }
}
