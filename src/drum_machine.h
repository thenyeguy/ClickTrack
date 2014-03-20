#ifndef DRUM_MACHINE_H
#define DRUM_MACHINE_H

#include <list>
#include <map>
#include <string>
#include "audio_generics.h"
#include "generic_instrument.h"
#include "basic_elements.h"
#include "scheduler.h"

namespace ClickTrack
{
    /* The following implements a basic drum machine. It triggers samples based
     * on a configuration file, called keymap.txt
     *
     * The config file consists of one sample per line in the following format:
     *      MidiNumber path/to/your sample.wav
     * Blank lines and lines starting with # are ignored.
     */
    class DrumVoice;
    class DrumMachine : public GenericInstrument
    {
        friend class DrumVoice;

        public:
            DrumMachine(const std::string& path);
            ~DrumMachine();

            Channel* get_output_channel();

            /* Reloads all voices from a new path
             */
            void set_voice(const std::string& path);

            /* Make the final gain filter public to expose volume changer
             */
            GainFilter volume;

        protected:
            /* The following callbacks are used to trigger and update the state
             * of our voices. They are entirely handled by this generic class.
             */
            void on_note_down(unsigned note, float velocity, unsigned long time=0);
            void on_note_up(unsigned note, float velocity, unsigned long time=0);
            void on_sustain_down(unsigned long time=0);
            void on_sustain_up(unsigned long time=0);
            void on_pitch_wheel(unsigned value, unsigned long time=0);

            /* Other MIDI messages vary from instrument to instrument. This can
             * be overriden to handle them
             */
            void on_midi_message(std::vector<unsigned char>* message,
                    unsigned long time=0);

            /* Sums our different voices
             */
            Adder adder;

        private:
            /* The following map simply maps MIDI numbers to their drum voice
             */
            std::map<unsigned, DrumVoice*> voices;
    };


    /* Drum voice manages a single sample in a drum machine. It reads its sample
     * into memory for playback. It will retrigger its sound on repeated note
     * down.
     */
    class DrumVoice : public AudioGenerator
    {
        friend class DrumMachine;

        public:
            DrumVoice(const std::string& filename);

        protected:
            /* Output function. Plays a wav file from buffer
             */
            void generate_outputs(std::vector<SAMPLE>& outputs, unsigned long t);

            /* Callback to trigger a note. Only called by a drum machine.
             *
             * The function is scheduled, and will use the function scheduler to
             * trigger the callback handler, which is passed the velocity as
             * a payload.
             */
            void on_note_down(unsigned note, float velocity, 
                    unsigned long time=0);
            static void handle_note_down(DrumVoice& caller, void* payload);

            /* Scheduler for note downs
             */
            FunctionScheduler<DrumVoice> scheduler;

            /* State for audio buffer
             */
            std::vector<SAMPLE> samples;
            unsigned next_i;
            bool playing;
    };


    class InvalidKeymap: public std::exception
    {
        public:
        InvalidKeymap(const std::string in_filename)
            : filename(in_filename) {}

        const std::string filename;
        virtual const char* what() const throw()
        {
            std::string message = 
                "DrumMachine could not open the specified keymap: " + filename;
            return message.c_str();
        }
    };
}

#endif
