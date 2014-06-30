#ifndef TIMING_MANGER_H
#define TIMING_MANGER_H

#include <chrono>
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

            /* An audio consumer may synchronize the timing manager. This
             * informs the manager that the specified sample time has been
             * written out at the time synchronize is called. This status is
             * stored with a timestamp, and returned by get_last_synchronziation
             */
            struct SynchronizationStatus
            {
                bool synced;
                unsigned long sample_time;
                std::chrono::time_point<std::chrono::high_resolution_clock,
                    std::chrono::duration<double> > timestamp;
            };

            void synchronize(unsigned long t);
            SynchronizationStatus get_last_synchronization();

        private:
            /* The next sample time to be processed
             */
            unsigned long time;

            /* A list of instruments and consumers that need processing
             */
            std::vector<AudioConsumer*> consumers;
            std::vector<GenericInstrument*> instruments;

            /* The last synchronization status
             */
            struct SynchronizationStatus last_sync;
    };
}

#endif
