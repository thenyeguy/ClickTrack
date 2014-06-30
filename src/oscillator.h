#ifndef OSCILLATOR_H
#define OSCILLATOR_H

#include "audio_generics.h"


namespace ClickTrack
{
    /* An oscillator is a basic generator unit. It handles periodic waveforms at
     * arbitrary frequencies, and supports many modes of operation
     */
    class Oscillator : public AudioGenerator
    {
        public:
            /* The oscillator supports many waveform modes.
             * Blep oscillators use PolyBlep to generate alias-free waveforms
             */
            enum Mode { Sine, Saw, Square, Tri, WhiteNoise, 
                BlepSaw, BlepSquare, BlepTri, PulseTrain};
            Oscillator(Mode mode, float in_freq);

            /* Sets the waveform mode
             */
            void set_mode(Mode mode);

            /* Sets the frequency; uses a function scheduler to trigger this
             * event at the specified time. If no time is given, applies
             * immediately
             */
            void set_freq(float freq);

            /* Given an increment in steps, transposes the output frequency of
             * the osciilator by that many steps
             */
            void set_transposition(float steps);

            /* The LFO adjusts the output waveform frequency by the specified step
             * degree; this can be fractional. If no LFO is specified, or if the
             * input is set to nullptr, no modulation is done.
             */
            void set_lfo_input(AudioChannel* input);
            void set_lfo_intensity(float steps);

            /* The modulator performs frequency modulation of the oscillator
             * with modulation index given by the intensity
             */
            void set_modulator_input(AudioChannel* input);
            void set_modulator_intensity(float intensity);

        private: 
            /* Overridden method for AudioGenerator to provide basic time
             * tracking and output for oscillators
             *
             * PolyBLEP oscillators use a periodic offset to remove aliasing
             */
            void generate_outputs(std::vector<SAMPLE>& outputs, unsigned long t);
            float polyBlepOffset(float t);
            float last_output; // used by blep triangle

            /* LFO input
             */
            AudioChannel* lfo;
            float lfo_intensity;

            /* Modulation input
             */
            AudioChannel* modulator;
            float mod_intensity;

            /* Phase state
             */
            float master_phase; // rads
            float phase_inc;    // rads
            float transpose;

            /* Oscillator state
             */
            Mode mode;
            float freq; // hz
    };
}

#endif
