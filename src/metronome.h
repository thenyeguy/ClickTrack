#ifndef METRONOME_H
#define METRONOME_H

#include <string>
#include "audio_generics.h"
#include "timing_manager.h"


namespace ClickTrack
{
    /* A metronome listens to the global meter and clicks on beats. It loads the
     * click sounds from specified wav files.
     */
    class Metronome : public AudioGenerator
    {
        public:
            Metronome(TimingManager& timing_manager,
                    const std::string& downbeat_sound,
                    const std::string& accented_sound,
                    const std::string& unaccented_sound);

        private:
            void generate_outputs(std::vector<SAMPLE>& output, unsigned long t);

            /* We need a pointer to the global metronome
             */
            RhythmManager& rhythm_manager;

            /* Load the click sounds into memory
             */ 
            std::vector<SAMPLE> downbeat;
            std::vector<SAMPLE> accented;
            std::vector<SAMPLE> unaccented;

            /* State for the current beat playing and sample index for it
             */
            unsigned current_sample;
            std::vector<SAMPLE>* current_beat;
    };
}


#endif
