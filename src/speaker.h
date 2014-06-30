#ifndef SPEAKER_H
#define SPEAKER_H

#include "audio_generics.h"
#include "portaudio_wrapper.h"
#include "timing_manager.h"


namespace ClickTrack
{
    /* The speaker is an output device. It uses the default output device on
     * your computer, and pushes its data out to portaudio.
     */
    class Speaker : public AudioConsumer
    {
        public:
            Speaker(TimingManager& timer, unsigned num_inputs = 1, 
                    bool defaultDevice=true);

        private:
            void process_inputs(std::vector<SAMPLE>& input, unsigned long t);

            /* Store our stream results
             */
            std::vector< std::vector<SAMPLE> > buffer;
            OutputStream stream;

            /* The TimingManager used for synchronization
             */
            TimingManager& timer;
    };
}


#endif
