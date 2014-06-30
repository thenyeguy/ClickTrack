#ifndef MIDI_WRAPPER_H
#define MIDI_WRAPPER_H

#include <chrono>
#include <rtmidi.h>
#include "generic_instrument.h"
#include "timing_manager.h"


namespace ClickTrack
{
    /* A wrapper for RtMIDI. It is registered directly to an instrument class
     * and handles all its playing through callbacks to the MIDI process. It
     * internally talks to its instrument to convey state.
     */
    class MidiListener
    {
        public:
            /* Constructor and destructor automatically open and close the
             * RtMidi streams for us.
             *
             * The MIDI listener requires an instrument to respond to in its
             * callback.
             *
             * If no channel is provided, the constructor asks which channel
             * to use.
             */
            MidiListener(TimingManager& timer, GenericInstrument& inst,
                    int channel=-1);

        private:
            /* Callback for registering with the input stream
             * Parses the MIDI message and passes on its message to the
             * specified destination.
             */
            static void midi_callback(double deltaTime,
                    std::vector<unsigned char>* message, void* in_listener);

            /* State for MIDI
             */
            RtMidiIn stream;
            GenericInstrument& inst;
            TimingManager& timer;
    };
}

#endif
