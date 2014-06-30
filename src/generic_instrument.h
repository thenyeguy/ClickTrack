#ifndef GENERIC_INSTRUMENT_H
#define GENERIC_INSTRUMENT_H

#include "audio_generics.h"
#include "midi_generics.h"


namespace ClickTrack
{
    /* The Generic Instrument is an abstract class that defines the interface
     * for a MIDI instrument class.
     */
    class GenericInstrument : public MidiConsumer
    {
        public:
            GenericInstrument();

            /* Exposes the final output channel of the instrument, so that it
             * may be plugged later into the signal chain.
             */
            AudioChannel* get_output_channel(int channel=0);
            const unsigned get_num_output_channels();

        protected:
            /* This function implements the logic for handling incoming MIDI
             * events. It calls the functions below.
             */
            void process_events(std::vector<MidiMessage>& inputs,
                    unsigned long t);

            /* The following functions are called by the midi consumer. They are
             * responsible for handling the messages sent to our instrument.
             * Must be overrideen.
             *
             * Some messages are parsed and identified by tis class. If so, they
             * use the named message types below. If a message is not special
             * handled, then the original message is passed directly to the
             * instrument using on_midi_message, for custom handling.
             *
             * All our handlers take a time to trigger, in sample time. A time
             * of zero means to trigger at the moment the message is received.
             */ 
            virtual void on_note_down(unsigned note, float velocity) = 0;
            virtual void on_note_up(unsigned note, float velocity) = 0;

            virtual void on_sustain_down() = 0;
            virtual void on_sustain_up() = 0;

            virtual void on_pitch_wheel(float value) = 0;
            virtual void on_modulation_wheel(float value) = 0;

            virtual void on_midi_message(MidiMessage message) = 0;

            /* Used by subclasses to add their own output channels. This must be
             * done during initialization of the subclass, and tells the
             * instrument where the final output is.
             */
            void add_output_channel(AudioChannel* channel);

        private:
            /* A vector of all our output channels.
             */
            std::vector<AudioChannel*> output_channels;
    };
}

#endif
