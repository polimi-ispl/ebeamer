/*
  ==============================================================================

    Beamformer.cpp
    Created: 14 Apr 2020 9:12:12am
    Author:  Luca Bondi

  ==============================================================================
*/

#include "Beamformer.h"

Beamformer::Beamformer(const AudioProcessor& p, int numBeams_, int numDoas_):processor(p){
    
    numBeams = numBeams_;
    numDoas = numDoas_;
    
    beamBuffer.setSize(numBeams, 0);
    
}

Beamformer::~Beamformer(){
    
}

void Beamformer::setBeamParameters(int beamIdx, const BeamParameters& beamParams){
    
}

void Beamformer::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock, int numActiveInputChannels){
    auto beamOutNumSamples = maximumExpectedSamplesPerBlock;
    beamBuffer.setSize(beamBuffer.getNumChannels(), beamOutNumSamples);
}

void Beamformer::processBlock(const AudioBuffer<float> &inBuffer){
    
    /** Simple passthrough */
    for (auto beamIdx = 0; beamIdx < numBeams; beamIdx++){
        beamBuffer.copyFrom(beamIdx, 0, inBuffer, beamIdx, 0, inBuffer.getNumSamples());
    }
}

void Beamformer::getBeams(AudioBuffer<float>& outBuffer){
    jassert(outBuffer.getNumChannels() == numBeams);
    jassert(outBuffer.getNumSamples() <= beamBuffer.getNumSamples() - beamBufferStartSample);
    for (auto beamIdx = 0; beamIdx < numBeams; beamIdx++){
        outBuffer.copyFrom(beamIdx, 0, beamBuffer, beamIdx, beamBufferStartSample, outBuffer.getNumSamples());
    }
}

void Beamformer::releaseResources(){
    
}
