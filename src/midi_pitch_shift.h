#ifndef MIDI_PITCH_SHIFT_H
#define MIDI_PITCH_SHIFT_H

#include "midi_generics.h"

namespace ClickTrack
{
    class MidiPitchShift : public MidiFilter
    {
        public:
            MidiPitchShift(int shift);

            void set_pitch_shift(int shift);

        private:
            void filter_events(std::vector<MidiMessage>& inputs, 
                    std::vector<MidiMessage>& outputs, unsigned long t);

            int shift;
    };
}

#endif
