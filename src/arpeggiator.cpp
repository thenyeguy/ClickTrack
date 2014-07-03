#include "arpeggiator.h"

using namespace ClickTrack;


Arpeggiator::Arpeggiator(TimingManager& timing_manager)
    : rhythm_manager(timing_manager.rhythm_manager),
      current_note(0),
      held_keys(),
      next_note(held_keys.end())
{}


void Arpeggiator::filter_events(std::vector<MidiMessage>& inputs,
        std::vector<MidiMessage>& outputs, unsigned long t)
{
    // First check for note ons/note offs
    for(auto input : inputs)
    {
        if(input.type == NOTE_DOWN)
        {
            // Insert into the list, sorted
            unsigned note = input.message[0];
            auto it = held_keys.begin();
            while(it != held_keys.end() && *it < note)
                it++;
            held_keys.insert(it, note);
        }

        if(input.type == NOTE_UP)
        {
            // Move the next note forward if they match, then remove
            unsigned note = input.message[0];
            if(next_note != held_keys.end() && *next_note == note)
                next_note++;
            held_keys.remove(note);
        }
    }

    // Next, check for on beat and trigger notes
    if(rhythm_manager.is_on_beat())
    {
        // First construct note off
        if(current_note != 0)
        {
            MidiMessage noteoff;
            noteoff.type = NOTE_UP;
            noteoff.channel = 0;
            noteoff.message.push_back(current_note);
            noteoff.message.push_back(0x7F);

            outputs.push_back(noteoff);
            current_note = 0;
        }

        // Then construct note on
        if(next_note == held_keys.end())
            next_note = held_keys.begin();

        if(held_keys.size() > 0)
        {
            current_note = *next_note;
            next_note++;

            MidiMessage noteon;
            noteon.type = NOTE_DOWN;
            noteon.channel = 0;
            noteon.message.push_back(current_note);
            noteon.message.push_back(0x7F);

            outputs.push_back(noteon);
        }
    }
}
