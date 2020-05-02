/*
  Midi CC definitions and classes
 
 Authors:
 Luca Bondi (luca.bondi@polimi.it)
*/

#pragma once

class MidiCC {
public:

    class Callback {
    public:

        virtual ~Callback() = default;

        /** Start learning the specified parameter */
        virtual void startCCLearning(const String &p) = 0;

        /** Stop learning the previous parameter */
        virtual void stopCCLearning() = 0;

        /** Get parameter being learned */
        virtual String getCCLearning() const = 0;

        /** Get a read-only reference to the parameters to CC mapping */
        virtual const std::map<String, MidiCC> &getParamToCCMapping() = 0;

        /** Remove mapping between MidiCC and parameter */
        virtual void removeCCParamMapping(const String &param) = 0;

    };

    bool operator<(const MidiCC &rhs) const {
        if (channel == rhs.channel) {
            return number < rhs.number;
        }
        return channel < rhs.channel;
    };

    int channel;
    int number;
};
