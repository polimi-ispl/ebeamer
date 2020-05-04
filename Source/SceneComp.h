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
    
    TileComp() {};
    
    ~TileComp() {};
    
    void paint(Graphics &) override;
    
    void resized() override {};
    
    void setColour(const Colour &col);
    
    void setPath(const Path &);
    
private:
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TileComp)
    
    Path path;
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
        
        virtual void getDoaEnergy(Mtx &energy) const = 0;
    };
    
    void setCallback(const Callback *p);
    
    void setParams(const std::atomic<float> *config,
                   const std::atomic<float> *frontFacing);
    
private:
    
    SpinLock lock;
    
    Rectangle<int> area;
    
    std::vector<std::vector<std::unique_ptr<TileComp>>> tiles;
    std::vector<std::vector<juce::Point<float>>> vertices;
    
    const Callback *callback = nullptr;
    const std::atomic<float> *frontFacingParam = nullptr;
    const std::atomic<float> *configParam = nullptr;
    
    std::vector<float> th;
    
    Mtx energy, energyPreGain;
    float inertia = 0.85;
    float gain = 0;
    const float maxGain = 60, minGain = -20;
    
    const float gridUpdateFrequency = 10;
    
    void makeLayout();
    
    void timerCallback() override;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GridComp)
};

//==============================================================================

class BeamComp : public Component {
public:
    BeamComp() {};
    
    ~BeamComp() {};
    
    void paint(Graphics &) override;
    
    void resized() override;
    
    void setParams(
                   const std::atomic<float> *config,
                   const std::atomic<float> *frontFacing,
                   const std::atomic<float> *mute,
                   const std::atomic<float> *width,
                   const std::atomic<float> *steerX,
                   const std::atomic<float> *steerY);
    
    //TODO: Use LookAndFeel
    void setBaseColor(Colour colour) { baseColour = colour; }
    
private:
    
    const std::atomic<float> *frontFacingParam = nullptr;
    const std::atomic<float> *muteParam = nullptr;
    const std::atomic<float> *widthParam = nullptr;
    const std::atomic<float> *steerXParam = nullptr;
    const std::atomic<float> *steerYParam = nullptr;
    const std::atomic<float> *configParam = nullptr;
    
    Rectangle<int> area;
    
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
        
        virtual const std::atomic<float> *getConfigParam() const = 0;
        
        virtual const std::atomic<float> *getFrontFacingParam() const = 0;
        
        virtual const std::atomic<float> *getBeamMute(int idx) const = 0;
        
        virtual const std::atomic<float> *getBeamWidth(int idx) const = 0;
        
        virtual const std::atomic<float> *getBeamSteerX(int idx) const = 0;
        
        virtual const std::atomic<float> *getBeamSteerY(int idx) const = 0;
    };
    
    void setCallback(const Callback *c);
    
    void paint(Graphics &) override;
    
    void resized() override;
    
    //TODO: Use LookAndFeel
    void setBeamColors(const std::vector<Colour> &colours);
    
    
private:
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SceneComp)
    const Callback *callback = nullptr;
    BeamComp beams[NUM_BEAMS];
    GridComp grid;
};
