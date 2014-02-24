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
    typedef short OPENSLES_SAMPLE;
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
            static OpenSlesWrapper& getInstance();

            /* Write outputs will send audio out to the system
             * speakers/headphones. Currently it supports only mono or stereo.
             */
            void writeOutputs(std::vector< std::vector<SAMPLE> >& outputs);

            /* Read inputs will grab the audio out from the system microphone.
             * Currently it only supports mono
             */
            void readInputs(std::vector< std::vector<SAMPLE> >& inputs);

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
            static void outputCallback(SLAndroidSimpleBufferQueueItf bq, 
                    void *context);

            const unsigned num_output_channels;
            OPENSLES_SAMPLE* outputBuffer;
            std::mutex outputLock;
            
            /* This callback is registered to the input buffer, and gets called
             * when the buffer is filled. The context will be a pointer back to
             * this object
             *
             * A mutex is used to lock the bufer until ready to read
             */
            static void inputCallback(SLAndroidSimpleBufferQueueItf bq, 
                    void *context);
            
            const unsigned num_input_channels;
            OPENSLES_SAMPLE* inputBuffer;
            std::mutex inputLock;

            /* Used internally, it is called when an OpenSL call fails to log
             * relevant information and die.
             *
             * If given a sucess, it NOPs
             */ 
            void handleOpenSlesError(SLresult result);

            /* The OpenSL ES engine is used to create all other classes
             */
            SLObjectItf engineObject;
            SLEngineItf engine;

            /* The output mix is responsible for controlling the actual audio
             * output. The player and its buffer are responsinble for putting
             * output to the OS
             */
            SLObjectItf outputMixObject;
            SLObjectItf playerObject;
            SLPlayItf player;
            SLAndroidSimpleBufferQueueItf outputBufferQueue;

            /* The input system is responsible for reading in audio
             */
            SLObjectItf recorderObject;
            SLRecordItf recorder;
            SLAndroidSimpleBufferQueueItf inputBufferQueue;
    };
}

#endif
