/*
 Acoustic scene components
 
 Authors:
 Matteo Scerbo (matteo.scerbo@mail.polimi.it)
 Luca Bondi (luca.bondi@polimi.it)
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "ebeamerDefs.h"
#include "MeterComp.h"
#include "Beamformer.h"

class TileComp : public Component {
public:

    TileComp();

    ~TileComp() {};

    void paint(Graphics &) override;

    void resized() override {};

    void setFrontFacingParam(const std::atomic<float> *p);

    void setColour(const Colour &col);

    void setCorners(const juce::Point<float> &,
                    const juce::Point<float> &,
                    const juce::Point<float> &,
                    const juce::Point<float> &);

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TileComp)

    juce::Point<float> corners[2][2];
    Colour tileColour;

    const std::atomic<float> *frontFacing = nullptr;
    AffineTransform frontFacingTransf;
};

//==============================================================================

class GridComp : public Component, public Timer {
public:
    GridComp();

    ~GridComp() {};

    void resized() override;

    class Callback {
    public:
        virtual ~Callback() = default;

        virtual void getDoaEnergy(std::vector<float> &energy) const = 0;
    };

    void setCallback(const Callback *p);

    void setParams(const std::atomic<float> *frontFacing);

private:

    TileComp tiles[TILE_ROW_COUNT][NUM_DOAS];
    juce::Point<float> vertices[TILE_ROW_COUNT + 1][NUM_DOAS + 1];

    const Callback *callback = nullptr;

    std::vector<float> th;

    std::vector<float> energy, energyPreGain;
    float inertia = 0.85;
    float gain = 0;
    const float maxGain = 60, minGain = -20;

    const float gridUpdateFrequency = 10;

    void computeVertices();

    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GridComp)
};

//==============================================================================

class BeamComp : public Component {
public:
    BeamComp() {};

    ~BeamComp() {};

    void paint(Graphics &) override;

    void resized() override {};

    void setParams(const std::atomic<float> *frontFacing,
                   const std::atomic<float> *mute,
                   const std::atomic<float> *width,
                   const std::atomic<float> *steer);

    //TODO: Use LookAndFeel
    void setBaseColor(Colour colour) { baseColour = colour; }

private:

    const std::atomic<float> *frontFacingParam = nullptr;
    const std::atomic<float> *muteParam = nullptr;
    const std::atomic<float> *widthParam = nullptr;
    const std::atomic<float> *steerParam = nullptr;


    Colour baseColour = Colours::lightblue;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BeamComp)
};

//==============================================================================

class SceneComp : public Component {
public:
    SceneComp();

    ~SceneComp() {};

    class Callback : public GridComp::Callback {
    public:
        virtual ~Callback() = default;

        virtual const std::atomic<float> *getFrontFacingParam() const = 0;

        virtual const std::atomic<float> *getBeamMute(int idx) const = 0;

        virtual const std::atomic<float> *getBeamWidth(int idx) const = 0;

        virtual const std::atomic<float> *getBeamSteer(int idx) const = 0;
    };

    void setCallback(const Callback *c);

    void paint(Graphics &) override;

    void resized() override;

    //TODO: Use LookAndFeel
    void setBeamColors(const std::vector<Colour> &colours);


private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SceneComp)

    BeamComp beams[NUM_BEAMS];
    GridComp grid;
};
