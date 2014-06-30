#include "midi_pitch_shift.h"

using namespace ClickTrack;


MidiPitchShift::MidiPitchShift(int in_shift)
    : MidiFilter(), shift(in_shift)
{}


void MidiPitchShift::set_pitch_shift(int in_shift)
{
    shift = in_shift;
}


void MidiPitchShift::filter_events(std::vector<MidiMessage>& inputs,
        std::vector<MidiMessage>& outputs, unsigned long t)
{
    for(auto input : inputs)
    {
        // If we have a note up/note down event, adjust its note number
        if(input.type == NOTE_DOWN || input.type == NOTE_UP)
        {
            int note = input.message[0] + shift;
            if(note < 0) note = 0;
            if(note > 127) note = 127;
            input.message[0] = note;
        }

        outputs.push_back(input);
    }
}
