#ifndef TIMING_MANGER_H
#define TIMING_MANGER_H

#include <vector>
#include "audio_generics.h"
#include "generic_instrument.h"

namespace ClickTrack
{
    /* A timing manager is used to run the entire processing chain. It has
     * a list of all audio consumers that must be called to process the audio,
     * and all instruments that must be called process MIDI inputs.
     */
    class TimingManager
    {
        public:
            TimingManager();

            /* The timing manager must be given references to all the
             * instruments and audio consumers during initialization in the
             * signal chain in order to process them.
             */
            void add_instrument(GenericInstrument* instrument);
            void add_consumer(AudioConsumer* consumer);

            /* Used to tick the processing one time step forward
             */
            void tick();

            /* Returns the next time step to be processed
             */
            unsigned long get_current_time();

        private:
            std::vector<AudioConsumer*> consumers;
            std::vector<GenericInstrument*> instruments;

            unsigned long time;
    };
}

#endif
