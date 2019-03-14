#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "DOAthread.h"

#define PLANAR_MODE

#define PERSPECTIVE_RATIO 5

#define TILE_ROW_COUNT 10
#define TILE_COL_COUNT 25
#define PI 3.14159265

#define GUI_WIDTH 540
#define SCENE_WIDTH 460

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
#define LEFT_RIGHT_MARGIN 20
#define TOP_BOTTOM_MARGIN 20
#define KNOBS_LEFT_RIGHT_MARGIN 20

#ifdef PLANAR_MODE
#define GUI_HEIGHT 750
#define SCENE_HEIGHT 230
#else
#define GUI_HEIGHT 980
#define SCENE_HEIGHT 460
#endif

//==============================================================================
class DecibelSlider : public Slider
{
public:
    DecibelSlider() {}

    double getValueFromText (const String& text) override
    {
        auto minusInfinitydB = -100.0;

        auto decibelText = text.upToFirstOccurrenceOf ("dB", false, false).trim();    // [1]

        return decibelText.equalsIgnoreCase ("-INF") ? minusInfinitydB
        : decibelText.getDoubleValue();  // [2]
    }

    String getTextFromValue (double value) override
    {
        return Decibels::toString (value,1);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DecibelSlider)
};

//==============================================================================

class TileComponent    : public Component
{
public:

    Point<float> corners[2][2];
    Colour tileColour;

    TileComponent();
    ~TileComponent();

    void paint(Graphics&) override;
    void resized() override;

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TileComponent)
};

//==============================================================================

class GridComponent    : public Component
{
public:
    GridComponent();
    ~GridComponent();

    void resized() override;

    void updateEnergy(std::vector<float>);

private:

    TileComponent tiles[TILE_ROW_COUNT][TILE_COL_COUNT];
    Point<float> vertices[TILE_ROW_COUNT+1][TILE_COL_COUNT+1];

    void computeVertices();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GridComponent)
};

//==============================================================================

class BeamComponent    : public Component
{
public:
    BeamComponent();
    ~BeamComponent();

    void paint(Graphics&) override;
    void resized() override;

    void move(float);
    void scale(float);

private:

    float position;
    float width;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BeamComponent)
};

//==============================================================================

class SceneComponent    : public Component
{
public:
    SceneComponent();
    ~SceneComponent();

    void paint(Graphics&) override;
    void resized() override;

    void updateEnergy(std::vector<float>);

private:

    GridComponent grid;
    BeamComponent beams[NUM_BEAMS];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SceneComponent)
};

//==============================================================================

class JucebeamAudioProcessorEditor  : public AudioProcessorEditor,
                                      private ToggleButton::Listener,
                                      private Slider::Listener,
                                      private HighResolutionTimer
{
public:

    JucebeamAudioProcessorEditor (JucebeamAudioProcessor&);
    ~JucebeamAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    JucebeamAudioProcessor& processor;
    DOAthread *DOAt;

    // Project specific

    SceneComponent scene;

    Slider steeringBeam1Slider;
    Slider steeringBeam2Slider;

    Label steerLabel;

    Label widthLabel;
    Slider widthBeam1Knob;
    Slider widthBeam2Knob;

    Label panLabel;
    Slider panBeam1Knob;
    Slider panBeam2Knob;

    Label gainLabel;
    DecibelSlider gainBeam1Knob;
    DecibelSlider gainBeam2Knob;

    Label muteLabel;
    TextButton beam1MuteButton;
    TextButton beam2MuteButton;

    // TODO: Meters


    // Callbacks
    void buttonClicked(Button *button) override;
    void sliderValueChanged(Slider *slider) override;
    void hiResTimerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JucebeamAudioProcessorEditor);
};
