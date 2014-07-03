#ifndef TIMING_MANGER_H
#define TIMING_MANGER_H

#include <chrono>
#include <vector>
#include "audio_generics.h"
#include "generic_instrument.h"
#include "rhythm_manager.h"

namespace ClickTrack
{
    /* A timing manager is used to run the entire processing chain. It has
     * a list of all audio consumers that must be called to process the audio,
     * and all instruments that must be called process MIDI inputs.
     *
     * The timing manager handles synchronization with the sound driver, to
     * track when writes are happening.
     *
     * The timing manager tracks tempo and timing within measures of music
     */
    class TimingManager
    {
        public:
            TimingManager();

            /* The timing manager must be given references to all the
             * instruments and audio consumers during initialization in the
             * signal chain in order to process them.
             */
            void add_midi_consumer(MidiConsumer* consumer);
            void add_audio_consumer(AudioConsumer* consumer);

            /* Used to tick the processing one time step forward
             */
            void tick();

            /* Returns the next time step to be processed
             */
            unsigned long get_current_time();


            /* The timing manager contains a time signature class used to track
             * the current meter and beat status. It is public so that its API
             * is exposed directly.
             */
            RhythmManager rhythm_manager;


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

            /* Lists of consumers that need processing
             */
            std::vector<MidiConsumer*> midi_consumers;
            std::vector<AudioConsumer*> audio_consumers;

            /* The last synchronization status
             */
            struct SynchronizationStatus last_sync;
    };
}

#endif
