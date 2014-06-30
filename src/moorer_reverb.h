#ifndef MOORER_REVERB_H
#define MOORER_REVERB_H

#include "audio_generics.h"
#include "ringbuffer.h"


namespace ClickTrack
{
    /* A simple algorithmic reverb based on work by Moorer
     * For explanation of the filter design, see "About This Reverberation
     * Business" by James Moorer.
     *
     * The room selection lets you change the reverb among a set of presets.
     *      HALL: Based on Boston Symphony Hall, by Moorer
     */
    class MoorerReverb : public AudioFilter
    {
        public:
            /* room is chosen from the enum below.
             * rev_time is the -60dB time, in milliseconds
             * gain is the gain on the entire reverb, in raw amplitude
             * wetness is the percent reverb in the signal, between 0 and 1
             */
            enum Room { HALL };
            MoorerReverb(Room room, float rev_time, float gain, float wetness,
                         unsigned num_channels);

            /* Setters for the reverb parameters. Currently, the room itself
             * can't be changed after initialization.
             *
             * Time is in seconds, gain is in decibels
             */
            void set_rev_time(float rev_time);
            void set_gain(float gain);
            void set_wetness(float wetness);

        private:
            /* The following are used to reinitialize coefficients when
             * parameters change
             */
            void set_comb_filter_gains();
            void allocate_ringbuffers();

            /* The filter function 
             */
            void filter(std::vector<SAMPLE>& input,
                    std::vector<SAMPLE>& output, unsigned long t);

            /* Filter parameters
             */
            Room room;
            float rev_time;
            float gain;
            float wetness;

            /* Tapped delay line. The delays are assumed to be in ascending
             * order, and the gains are matched to the delays.
             *
             * tapped_delay_lines is a vector of ringbuffers, one for each
             * output channel
             */
            std::vector< RingBuffer<SAMPLE> > tapped_delay_lines;
            std::vector<unsigned> tapped_delays;
            std::vector<float>    tapped_gains;

            /* Comb filter. The delays and gains are assumed to be matched.
             *
             * comb_delay_lines is a vector of vectors of ringbuffers. The outer
             * vector is for each channel, the inner vector is for each comb
             * filter
             */
            std::vector< std::vector< RingBuffer<SAMPLE> > > comb_delay_lines;
            std::vector<unsigned> comb_delays;
            std::vector<float>    comb_gains;

            /* Combination parameters
             *
             * The output buffers are vectors of ringbuffers, one for each
             * output channel
             */
            std::vector< RingBuffer<SAMPLE> > tapped_delay_line_outputs;
            std::vector< RingBuffer<SAMPLE> > comb_outputs;
            unsigned tapped_out_delay;
            unsigned comb_out_delay;
            float    comb_out_gain;
    };
}

#endif
