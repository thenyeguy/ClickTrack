#ifndef CONVOLUTION_REVERB_H
#define CONVOLUTION_REVERB_H

#include "adder.h"
#include "audio_generics.h"
#include "convolution_filter.h"
#include "gain_filter.h"


namespace ClickTrack
{
    /* A simple one channel convolution reverb filter.
     */
    class ConvolutionReverb : public FilterBank
    {
        public:
            ConvolutionReverb(unsigned impulse_length, SAMPLE* impulse,
                   float gain, float wetness);

            void set_input_channel(Channel* channel,
                    unsigned channel_i = 0);
            void remove_channel(unsigned channel_i = 0);

            unsigned get_channel_index(Channel* channel);

        private:
            float gain;
            float wetness;
            
            ConvolutionFilter conv; 
            GainFilter wet;
            GainFilter dry;
            Adder out;
    };


    /* Given a wav file, reads that wav file into a pair of arrays. The two
     * arrays represent the impulse response given in the left and right
     * channels.
     */
    typedef struct {
        unsigned num_samples;
        SAMPLE* left;
        SAMPLE* right;
    } impulse_pair;
    impulse_pair* impulse_from_wav(const char* filename);
}

#endif
