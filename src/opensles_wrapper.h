#ifndef OPENSLES_WRAPPER_H
#define OPENSLES_WRAPPER_H

#include <cstdlib>
#include <list>
#include <mutex>
#include <vector>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "opensl_io.h"



namespace ClickTrack
{
    /* Define the sample type and buffer sizes
     */
    typedef float SAMPLE;
    typedef signed short OPENSLES_SAMPLE;
    const unsigned SAMPLE_RATE = 44100;
    const unsigned BUFFER_SIZE = 512;


    /* The OpenSlesWrapper is a C++ wrapping class for the native sound library.
     * It in turn wraps a small C library that creates our microphone and
     * speaker connections.
     *
     * This class acts as a singleton.
     */
    class OpenSlesWrapper
    {
        public:
            /* This will return a reference to the singleton object, and
             * instantiate it if necessary
             */
            static OpenSlesWrapper& get_instance();

            /* Send audio out to the system speakers/headphones.
             * This call is blocking.
             */
            void write_outputs(std::vector< std::vector<SAMPLE> >& outputs);

            /* Grabs the audio out from the system microphone
             * This call is blocking.
             */
            void read_inputs(std::vector< std::vector<SAMPLE> >& inputs);


        private:
            /* Leave the constructor/destructor private to enforce singleton
             */
            OpenSlesWrapper();
            ~OpenSlesWrapper();

            /* The number of channels used for IO.
             */
            const unsigned num_channels;

            /* Our stream object and associated buffers connect us to audio
             */
            OPENSL_STREAM* stream;
            SAMPLE* output_buffer;
            SAMPLE* input_buffer;
    };
}

#endif
