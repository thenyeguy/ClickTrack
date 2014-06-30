#ifndef CONVOLUTION_FILTER_H
#define CONVOLUTION_FILTER_H

#include <vector>
#include "audio_generics.h"
#include "fft.h"
#include "ringbuffer.h"


namespace ClickTrack
{
    /* The ConvolutionFilter implements a real time convolution of the input
     * signal with a precomputed impulse response.
     *
     * Uses the overlap add method to keep a running buffer of the result.
     * Also uses overlap add to calculate the convolution of each time block
     * with the impulse response.
     *
     * Computes circular convolution using the property that convolution in time
     * is equivalent to multiplication in frequency. Uses linearity of the FFT
     * to only perform a single transform into and out of frequency domain.
     */
    const unsigned CONVOLUTION_BUFFER_SIZE = BUFFER_SIZE;
    class ConvolutionFilter : public AudioFilter
    {
        public:
            ConvolutionFilter(unsigned impulse_length,
                              SAMPLE* in_impulse_response,
                              float gain, float wetness);
            ~ConvolutionFilter();

            void set_gain(float in_gain);
            void set_wetness(float in_wetness);

        private:
            // Filter only queues up its input samples, then when that queue is
            // full it calls process
            void filter(std::vector<SAMPLE>& input, 
                    std::vector<SAMPLE>& output, unsigned long t);
            void process(unsigned long start_t);

            float gain;
            float wetness;

            const unsigned transform_size;
            Transformer transformer;

            const unsigned num_impulse_blocks;
            std::vector<std::complex<SAMPLE>*> impulse_response;

            RingBuffer<std::complex<SAMPLE>*> frequency_buffer;
            RingBuffer<SAMPLE> reverb_buffer;


            // Preallocate buffers for speed
            std::complex<SAMPLE>* input_buffer;
            std::complex<SAMPLE>* output_buffer;
            std::vector<SAMPLE> output_queue;
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
