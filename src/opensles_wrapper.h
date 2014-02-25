#ifndef OPENSLES_WRAPPER_H
#define OPENSLES_WRAPPER_H

#include <cstdlib>
#include <mutex>
#include <vector>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>



namespace ClickTrack
{
    /* Define the sample type and buffer sizes
     */
    typedef float SAMPLE;
    typedef signed short OPENSLES_SAMPLE;
    const unsigned SAMPLE_RATE = 44100;
    const unsigned BUFFER_SIZE = 128;


    /* The OpenSlesWrapper is a wrapping class for the native sound library. It
     * will initialized all the required boilerplate code. It behaves as
     * a singleton class.
     */
    class OpenSlesWrapper
    {
        public:
            /* This will return a reference to the singleton object, and
             * instantiate it if necessary
             */
            static OpenSlesWrapper& get_instance();

            /* Write outputs will send audio out to the system
             * speakers/headphones. Currently it supports only mono or stereo.
             */
            void write_outputs(std::vector< std::vector<SAMPLE> >& outputs);

            /* Read inputs will grab the audio out from the system microphone.
             * Currently it only supports mono
             */
            void read_inputs(std::vector< std::vector<SAMPLE> >& inputs);

        private:
            /* Constructor/destructor are made private as this is a singleton
             * class.
             */
            OpenSlesWrapper();
            ~OpenSlesWrapper();

            /* This callback is registered to the output buffer, and gets called
             * when the buffer is emptied. The context will be a pointer back to
             * this object
             *
             * A mutex is used to lock the buffer until ready to receive
             */
            static void output_callback(SLAndroidSimpleBufferQueueItf bq, 
                    void *context);
            std::mutex output_lock;

            static void input_callback(SLAndroidSimpleBufferQueueItf bq, 
                    void *context);
            std::mutex input_lock;

            OPENSLES_SAMPLE* output_buffer;
            OPENSLES_SAMPLE* input_buffer;

            /* Used internally, it is called when an OpenSL call fails to log
             * relevant information and die. Passed a string that provides
             * information as what the caller was
             *
             * If given a sucess, it NOPs
             */ 
            void check_error(const char* info, SLresult result);

            /* The OpenSL ES engine is used to create all other classes
             */
            const unsigned num_channels;
            SLObjectItf engine_object;
            SLEngineItf engine;

            /* The output mix is responsible for controlling the actual audio
             * output. The player and its buffer are responsinble for putting
             * output to the OS
             */
            SLObjectItf output_mix_object;
            SLObjectItf player_object;
            SLPlayItf player;
            SLAndroidSimpleBufferQueueItf output_buffer_queue;

            /* The input system is responsible for reading in audio
             */
            SLObjectItf recorder_object;
            SLRecordItf recorder;
            SLAndroidSimpleBufferQueueItf input_buffer_queue;
    };
}

#endif
