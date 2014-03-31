#ifndef MICROPHONE_H
#define MICROPHONE_H

#include "audio_generics.h"
#include "opensl_io.h"


namespace ClickTrack
{
    /* The microphone is an input device. It uses the default input device on
     * your computer, and pulls its data from the driver.
     */
    class Microphone : public AudioGenerator
    {
        public:
            Microphone(unsigned num_channels = 1);

        private:
            void generate_outputs(std::vector<SAMPLE>& output, unsigned long t);

            /* Store our stream results
             */
            std::vector< std::vector<SAMPLE> > buffer;
            OpenSlesWrapper& openSles;
    };
}


#endif
