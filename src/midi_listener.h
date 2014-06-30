#ifndef MIDI_WRAPPER_H
#define MIDI_WRAPPER_H

#include <chrono>
#include <queue>
#include <rtmidi.h>
#include "midi_generics.h"
#include "timing_manager.h"


namespace ClickTrack
{
    /* A wrapper for RtMIDI. It is registered directly to an instrument class
     * and handles all its playing through callbacks to the MIDI process. It
     * internally talks to its instrument to convey state.
     */
    class MidiListener : public MidiGenerator
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
            MidiListener(TimingManager& timer, int channel=-1);

        private:
            /* Generator function to pass out events
             */
            void generate_events(std::vector<MidiMessage>& outputs,
                    unsigned long t);

            /* Callback for registering with the input stream
             * Parses the MIDI message and passes on its message to the
             * specified destination.
             */
            static void midi_callback(double deltaTime,
                    std::vector<unsigned char>* message, void* in_listener);

            /* State for MIDI
             */
            RtMidiIn stream;
            TimingManager& timer;

            /* Internally, we map sample timestamps to their time, function,
             * and payload to be triggered. Events are ordered by their time and
             * stored in a priority queue. Attach an index to ensure first in
             * first out ordering
             */
            unsigned event_i;
            struct event_t { unsigned i; unsigned long t; MidiMessage m; };
            struct event_t_comp
            { 
                bool operator()(const struct event_t e1, const struct event_t e2)
                {
                    if(e1.t == e2.t)
                        return e1.i > e2.i;
                    return e1.t > e2.t;
                }
            };

            std::priority_queue<struct event_t, std::vector<struct event_t>,
                struct event_t_comp> events;
    };
}

#endif
