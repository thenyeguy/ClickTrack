#ifndef RHYTHM_MANAGER_H
#define RHYTHM_MANAGER_H

#include <vector>

namespace ClickTrack
{
    /* Rhythm manager is a class used by the timing manager to encapsulate state
     * related to beats in a measure of music.
     */
    class RhythmManager
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
             *
             * is_xxx_subdivision() takes in a multiple, and returns whether or
             *     not we fall directly on a subdivision samples.
             *     i.e. is_beat_subdivision(2) tells us whether we fall on an
             *     eighth note in 4/4, and is_beat_subdivision(3,2) tells us
             *     whether we fall on an eight note triplet
             */
            bool is_on_beat();
            unsigned get_current_beat();
            BeatType get_current_beat_type();

            bool is_beat_subdivision(unsigned numerator, 
                    unsigned denominator=1);
            bool is_measure_subdivision(unsigned numerator, 
                    unsigned denominator=1);

        private:
            /* Protect the constructor and tick function to limit access to
             * TimingManager
             */
            RhythmManager();
            void tick();

            /* Store state to update time counter and compute beats
             */
            unsigned tempo;
            unsigned samples_per_beat;

            /* Store state to track the current beat and the meter type.
             * current_tick indicates how many samples into a measure we are
             */
            std::vector<BeatType> meter;
            unsigned current_beat;
            unsigned current_tick;
    };
}

#endif
