#ifndef SUBTRACTIVE_SYNTH_H
#define SUBTRACTIVE_SYNTH_H

#include "adsr.h"
#include "gain_filter.h"
#include "oscillator.h"
#include "polyphonic_instrument.h"
#include "second_order_filter.h"


namespace ClickTrack
{
    /* This is a subtractive synthesizer controlled over MIDI. It is polyphonic.
     */
    class SubtractiveSynthVoice;
    class SubtractiveSynth : public PolyphonicInstrument
    {
        public:
            /* Constructor/destructor. Takes in how many voices to use, as
             * well as the MIDI channel
             */
            SubtractiveSynth(int voices=1);

            /* Override because we have added more signal chain
             */
            AudioChannel* get_output_channel();

            /* Sets the output gain of the synth, in decibels
             */
            void set_gain(float gain);

            /* Voice selection for each oscillator - each oscillator can be one
             * of several modes as defined in oscillator.h
             */
            void set_osc1_mode(Oscillator::Mode mode);
            void set_osc2_mode(Oscillator::Mode mode);

            /* Transposition selection for each oscillator - transpose in
             * arbitrary step intervals
             */
            void set_osc1_transposition(float steps);
            void set_osc2_transposition(float steps);

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
            std::vector<SubtractiveSynthVoice*> voices;
    };


    class SubtractiveSynthVoice : public PolyphonicVoice
    {
        friend class SubtractiveSynth;

        public:
            /* Constructor/destructor
             */
            SubtractiveSynthVoice(SubtractiveSynth* parent_synth);

            /* Gets the output of this voice
             */
            AudioChannel* get_output_channel();

            /* Callbacks for starting and stopping notes
             */
            void handle_note_down(float velocity);
            void handle_note_up();
            void handle_pitch_wheel(float value);

        protected:
            /* Define our signal chain
             */
            Oscillator osc1, osc2;
            
            Adder adder;
            ADSRFilter adsr;
    };
}


#endif
