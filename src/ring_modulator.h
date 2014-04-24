#ifndef RING_MODULATOR_H
#define RING_MODULATOR_H

#include "audio_generics.h"
#include "oscillator.h"


namespace ClickTrack
{
    /* The Ring Modulator multiplies its inputs by an oscillator output, by
     * default a sine wave
     */
    class RingModulator : public AudioFilter
    {
        public:
            RingModulator(float freq, float wetness, unsigned num_channels=1);

            Oscillator modulator;
            
        private:
            void filter(std::vector<SAMPLE>& input,
                    std::vector<SAMPLE>& output, unsigned long t);

            float wetness;
    };
}

#endif
