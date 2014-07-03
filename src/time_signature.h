#ifndef TIME_SIGNATURE_H
#define TIME_SIGNATURE_H

#include <vector>

namespace ClickTrack
{
    /* Time signature is a class used by the timing manager to encapsulate state
     * related to beats in a measure of music.
     */
    class TimeSignature
    {
        friend class TimingManager;

        public:
            /* The following functions get and set the tempo. Tempo is given in
             * beats per minute. Setting the tempo resets to the top of the
             * measure.
             *
             * The default tempo is 120bpm.
             */
            unsigned get_tempo();
            void set_tempo(unsigned tempo);

            /* A measure is composed of a series of beats. These can either be
             * a downbeat, an accented beat, or an unaccented beat.
             *
             * The default is a 4/4: DOWNBEAT UNACCENTED UNACCENTED UNACCENTED
             */
            enum BeatType { DOWNBEAT, ACCENTED, UNACCENTED };
            typedef std::vector<BeatType> meter_t;

            void set_meter(meter_t& meter);
            const meter_t& get_current_meter();

            /* The following functions are used to get the current state of the
             * measure. 
             *
             * is_on_beat() returns true if the current sample is on a beat
             * get_current_beat() returns the zero indexed count of what beat we
             *     are currently playing
             * get_current_beat_type() returns the type of beat we are
             *     currently playing
             */
            bool is_on_beat();
            unsigned get_current_beat();
            BeatType get_current_beat_type();

        protected:
            /* Protect the constructor and tick function to limit access to
             * TimingManager
             */
            TimeSignature();
            void tick();

        private:
            /* Store state to update time counter and compute beats
             * current_tick indicates how many samples into a beat we are
             */
            unsigned tempo;
            unsigned samples_per_beat;
            unsigned current_tick;

            /* Store state to track the current beat and the meter type.
             */
            std::vector<BeatType> meter;
            unsigned current_beat;
    };
}

#endif
