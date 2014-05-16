#ifndef FM_SYNTH_H
#define FM_SYNTH_H

#include "adsr.h"
#include "gain_filter.h"
#include "oscillator.h"
#include "polyphonic_instrument.h"
#include "second_order_filter.h"


namespace ClickTrack
{
    /* This is a subtractive synthesizer controlled over MIDI. It is polyphonic.
     */
    class FMSynthVoice;
    class FMSynth : public PolyphonicInstrument
    {
        public:
            /* Constructor/destructor. Takes in how many voices to use, as
             * well as the MIDI channel
             */
            FMSynth(int voices=1);

            /* Override because we have added more signal chain
             */
            Channel* get_output_channel();

            /* Sets the output gain of the synth, in decibels
             */
            void set_gain(float gain);

            /* Voice selection for each oscillator - each oscillator can be one
             * of several modes as defined in oscillator.h
             */
            void set_carrier_mode(Oscillator::Mode mode);
            void set_modulator_mode(Oscillator::Mode mode);

            /* Transposition selection for each oscillator - transpose in
             * arbitrary step intervals
             */
            void set_carrier_transposition(float steps);
            void set_modulator_transposition(float steps);

            /* Controls the degree of modulation
             */
            void set_modulator_intensity(float intensity);

            /* Setters for the ADSR state
             */
            void set_attack_time(float attack_time);
            void set_decay_time(float decay_time);
            void set_sustain_level(float sustain_level);
            void set_release_time(float release_time);

            /* A filter in the signal chain
             */
            SecondOrderFilter filter;

            /* The LFO is connected to the oscillators to generate vibrato, and
             * to the output gain to generate tremolo.
             *
             * Vibrato is specified in steps, tremolo in decibal variation
             */
            Oscillator lfo;
            void set_lfo_vibrato(float steps);
            void set_lfo_tremolo(float db);

        private:
            /* Output gain for the oscillator. Also performs tremolo
             */
            GainFilter volume;

            /* Our list of voices so we may set their parameters
             */
            std::vector<FMSynthVoice*> voices;
    };


    class FMSynthVoice : public PolyphonicVoice
    {
        friend class FMSynth;

        public:
            /* Constructor/destructor
             */
            FMSynthVoice(FMSynth* parent_synth);

            /* Gets the output of this voice
             */
            Channel* get_output_channel();

            /* Callbacks for starting and stopping notes
             */
            void handle_note_down(float velocity, unsigned long time);
            void handle_note_up(unsigned long time);
            void handle_pitch_wheel(float value, unsigned long time);

        protected:
            /* Define our signal chain
             */
            Oscillator carrier, modulator;
            ADSRFilter adsr;
    };
}


#endif
