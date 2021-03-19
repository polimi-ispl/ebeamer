/*
  Project-wise types and definitions
 
 Authors:
 Luca Bondi (luca.bondi@polimi.it)
*/

#pragma once

#define NUM_BEAMS 2
#define NUM_DOAX 25
#define NUM_DOAY 9

#define GUI_WIDTH 540
#define GUI_HEIGHT 830

#define SCENE_WIDTH 460
#define SCENE_HEIGHT 230

#define MEDIUM_MARGIN 5

#define ULA_TILE_ROW_COUNT 7

#define LABEL_BEAM_WIDTH 25
#define STEER_SLIDER_HEIGHT 40
#define STEER_SLIDER_TOP_MARGIN 10
#define KNOB_WIDTH 150
#define KNOB_HEIGHT 80
#define KNOB_TOP_MARGIN 8
#define MUTE_HEIGHT 40
#define MUTE_WIDTH 40
#define MUTE_LEFT_RIGHT_MARGIN 20
#define MUTE_TOP_MARGIN 8
#define LABEL_WIDTH 70
#define LABEL_HEIGHT 20
#define LEFT_RIGHT_MARGIN 10
#define TOP_BOTTOM_MARGIN 20
#define KNOBS_LEFT_RIGHT_MARGIN 20
#define BEAM_LED_WIDTH 5
#define BEAM_TOP_BOTTOM_MARGIN 10
#define BEAM_LEFT_RIGHT_MARGIN 10

#define INPUT_SECTION_TOP_MARGIN 20
#define INPUT_HPF_SLIDER_HEIGHT 40
#define INPUT_HPF_LABEL_WIDTH 50
#define INPUT_LEFT_RIGHT_MARGIN 10
#define INPUT_LED_HEIGHT 5
#define INPUT_GAIN_SLIDER_HEIGHT 40
#define INPUT_GAIN_LABEL_WIDTH 50

#define FOOTER_MARGIN 10
#define FOOTER_HEIGHT 20
#define CPULOAD_WIDTH 100
#define CPULOAD_UPDATE_FREQ 10 //Hz

#define FRONT_TOGGLE_LABEL_WIDTH 65
#define FRONT_TOGGLE_WIDTH 25

#define CONFIG_COMBO_LABEL_WIDTH 60
#define CONFIG_COMBO_WIDTH 105

#define OSC_LED_SIZE 5
#define OSC_PORT_LABEL_WIDTH 65
#define OSC_PORT_WIDTH 50

#define INPUT_METER_UPDATE_FREQ 10 //Hz
#define BEAM_METER_UPDATE_FREQ 10 //Hz
#define ENERGY_UPDATE_FREQ 10 //Hz

/** Uncomment to compile in headless mode, without GUI and DOA estimation */
//#define HEADLESS

#include "../JuceLibraryCode/JuceHeader.h"

/** Available eSticks configurations type */
typedef enum {
    ULA_1ESTICK,
    ULA_2ESTICK,
    ULA_3ESTICK,
    ULA_4ESTICK,
    URA_2ESTICK,
    URA_3ESTICK,
    URA_4ESTICK,
    URA_2x2ESTICK,
} MicConfig;

/** Available eSticks configurations labels */
const StringArray micConfigLabels({
                                          "Single",
                                          "Horiz 2",
                                          "Horiz 3",
                                          "Horiz 4",
                                          "Stack 2",
                                          "Stack 3",
                                          "Stack 4",
                                          "Stack 2x2",
                                  });

bool isLinearArray(MicConfig m);
