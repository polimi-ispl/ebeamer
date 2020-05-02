/*
 Base classes for GUI components used with MIDI CC
 
 Authors:
 Luca Bondi (luca.bondi@polimi.it)
*/

#include <JuceHeader.h>
#include "MidiComp.h"

//==============================================================================

MidiCCPopup::MidiCCPopup(Component &owner_) : owner(owner_) {

}

MidiCCPopup::~MidiCCPopup() {

}

void MidiCCPopup::setCallback(MidiCC::Callback *cb, const String &param) {
    callback = cb;
    paramName = param;
}

bool MidiCCPopup::isLearning() const {
    return paramName == callback->getCCLearning();
}

void MidiCCPopup::showPopupMenu() {
    PopupMenu m;
    m.setLookAndFeel(&owner.getLookAndFeel());
    if (isLearning()) {
        m.addItem(1, "Stop learning CC", true, false);
    } else {
        m.addItem(1, "Learn CC", true, false);
    }

    auto mapping = callback->getParamToCCMapping();
    if (mapping.count(paramName) > 0) {
        m.addItem(2, "Forget CC ", true, false);
        m.addItem(3, "Chan: " + String(mapping[paramName].channel) + " - CC: " + String(mapping[paramName].number),
                  false, false);
    }

    if (popupArea.getX() == 0) {
        popupArea = PopupMenu::Options().getTargetScreenArea();
    }
    m.showAt(popupArea, 0, 0, 0, 0, ModalCallbackFunction::create(sliderMenuCallback, this));

}

void MidiCCPopup::sliderMenuCallback(int result, MidiCCPopup *popup) {
    if (popup != nullptr) {
        switch (result) {
            case 1:
                if (!popup->isLearning()) {
                    popup->callback->removeCCParamMapping(popup->paramName);
                    popup->callback->startCCLearning(popup->paramName);
                    popup->showPopupMenu();
                } else {
                    popup->callback->stopCCLearning();
                    popup->popupArea.setBounds(0, 0, 0, 0);
                }
                break;
            case 2:
                popup->callback->removeCCParamMapping(popup->paramName);
                popup->popupArea.setBounds(0, 0, 0, 0);
                break;
            default:
                break;
        }
    }
}
