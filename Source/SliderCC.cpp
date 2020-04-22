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

SliderCC::SliderCC(){
}


SliderCC::~SliderCC(){
    
}

void SliderCC::setProcessorParamName(EbeamerAudioProcessor* proc, const String & param){
    processor = proc;
    paramName = param;
}

bool SliderCC::isLearning() const{
    return paramName == processor->getCCLearning();
}

void SliderCC::showPopupMenu()
{
    PopupMenu m;
    m.setLookAndFeel (&getLookAndFeel());
    m.addItem(1,"Learn CC",true,isLearning());
    
    auto mapping = processor->getParamToCCMapping();
    if (mapping.count(paramName) > 0){
        m.addItem(2,"Forget CC " + String(mapping[paramName].channel) + ":" + String(mapping[paramName].number),true,false);
    }

    if (popupArea.getX() == 0){
        popupArea = PopupMenu::Options().getTargetScreenArea();
    }
    m.showAt(popupArea,0,0,0,0,ModalCallbackFunction::forComponent (sliderMenuCallback, this));
    
}

void SliderCC::sliderMenuCallback (int result, SliderCC* slider)
{
    if (slider != nullptr)
    {
        switch (result)
        {
            case 1:
                if (!slider->isLearning()){
                    slider->processor->startCCLearning(slider->paramName);
                    slider->showPopupMenu();
                }else{
                    slider->processor->stopCCLearning();
                    slider->popupArea.setBounds(0,0,0,0);
                }
                break;
            case 2:
                slider->processor->removeCCParamMapping(slider->paramName);
                slider->popupArea.setBounds(0,0,0,0);
                break;
            default:  break;
        }
    }
}

void SliderCC::mouseDown (const MouseEvent& e){
    if (e.mods.isPopupMenu()){
        showPopupMenu();
    }else{
        Slider::mouseDown(e);
    }
}
