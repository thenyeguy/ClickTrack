#ifndef ARPEGGIATOR_H
#define ARPEGGIATOR_H

#include <list>
#include "midi_generics.h"
#include "timing_manager.h"

namespace ClickTrack
{
    /* An arpeggiator listens to the midi keys being held, and walks up them one
     * at a time, synced with the beats of the global metronome.
     */
    class Arpeggiator : public MidiFilter
    {
        public:
            Arpeggiator(TimingManager& timing_manager);

        private:
            void filter_events(std::vector<MidiMessage>& inputs,
                    std::vector<MidiMessage>& outputs, unsigned long t);

            /* The rhythm manager is used to tick notes forward on beat
             */
            RhythmManager& rhythm_manager;

            /* Used to track the currently held keys, and our position in the
             * arpeggio. held_keys is always a sorted list of midi note numbers
             */
            unsigned current_note;
            std::list<unsigned> held_keys;
            std::list<unsigned>::iterator next_note;
    };
}

#endif
