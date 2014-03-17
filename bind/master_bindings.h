#include <chrono>
#include <jni.h>
#include "../src/basic_elements.h"
#include "../src/io_elements.h"
#include "../src/opensles_wrapper.h"
#include "../src/oscillator.h"
#include "../src/subtractive_synth.h"


namespace ClickTrack
{
    /* This class is a master container for the ClickTrack processing backend in
     * C++. It is responsible for containing all the global state in the
     * native code end.
     *
     * All members and methods are kept public in this class. This is so all
     * wrapping methods can instead be wrote directly into exported C functions
     * to avoid writing duplicate code.
     *
     * The one exception is the constructor/destructor. These are managed so as
     * to enforce a singleton class. This class can thus be accessed as a global
     * from anywhere in the C++ backend without a pointer reference, and only
     * one copy can be created - important because the audio driver is itself
     * only accessible from one instance
     */
    class ClickTrackMaster
    {
        public:
            /* Return an instance to the singleton object
             */
            static ClickTrackMaster& get_instance();

            /* These functions will start up and stop the OpenSL ES stream
             * objects.
             *
             * Note that it is nessecary to stop the object to prevent 
             * ClickTrack from holding a wakelock.
             *
             * Also note that the processing is NOT automatically started, and
             * must be manually called
             */
            void start();
            void stop();

            /* The play/pause functions will play/pause the audio output. They
             * can safely be called at any time, regardless of the current
             * state.
             *
             * Note that these do not shut down the audio processing, and will 
             * maintaina wakelock
             */
            void play();
            void pause();

            /* The following functions are used for tracking time and buffer
             * offsets. The consumer callback is registered with our output
             * speaker, and passes this object's pointer as its payload
             *
             * get_timestamp() returns the proper sample timestamp for an event
             * triggered at the current moment
             */
            static void consumer_callback(unsigned long time, void* payload);
            unsigned long get_timestamp();

            std::chrono::time_point<std::chrono::high_resolution_clock,
                std::chrono::duration<double> > buffer_timestamp;
            unsigned long next_buffer_time;

            /* These track the current state of our global playback
             */
            enum MasterState { PAUSED, PAUSING, PLAYING };
            MasterState state;

            /* Our signal chain. These elements are accessed directly by the JNI
             * wrapping functions
             */
            OpenSlesWrapper& openSles;
            Microphone microphone;
            GainFilter mic_gain;

            SawWave osc;
            GainFilter osc_gain;

            SubtractiveSynth sub_synth;
            GainFilter sub_synth_gain;

            Adder master_adder;
            Speaker speaker;

        protected:
            /* Constructor/destructor handles our entire audio chain. Protected
             * to enforce singleton
             */
            ClickTrackMaster();
            ~ClickTrackMaster();
    };
}


/* The following bindings are wrappers for the ClickTrackMaster class. They all
 * reference the singleton class automagically.
 */
// Concatenate the package name on our functions
#define PACKAGE(f) Java_edu_cmu_ece_ece551_clicktrack_ClickTrack_##f
#define SUBSYNTH(f) Java_edu_cmu_ece_ece551_clicktrack_ClickTrack_00024SubtractiveSynth_##f
extern "C"
{
    /* These functions start and stop the audio processing. Note that unless the
     * processing is stopped, OpenSLES will maintain a wakelock
     */
    JNIEXPORT void JNICALL PACKAGE(start)(
                JNIEnv* jenv, jobject jobj);
    JNIEXPORT void JNICALL PACKAGE(stop)(
                JNIEnv* jenv, jobject jobj);

    /* This is a wrapper for ClickTrackMaster::play(). This call spawns another
     * thread and returns after
     */
    JNIEXPORT void JNICALL PACKAGE(play)(
                JNIEnv* jenv, jobject jobj);

    /* This is a wrapper for ClickTrackMaster::pause()
     */
    JNIEXPORT void JNICALL PACKAGE(pause)(
                JNIEnv* jenv, jobject jobj);

    /* These set the gain on our components
     */
    JNIEXPORT void JNICALL PACKAGE(setMicGain)(
                JNIEnv* jenv, jobject jobj, jfloat gain);
    JNIEXPORT void JNICALL PACKAGE(setOscGain)(
                JNIEnv* jenv, jobject jobj, jfloat gain);
    JNIEXPORT void JNICALL SUBSYNTH(setGain)(
                JNIEnv* jenv, jobject jobj, jfloat gain);

    /* Play notes on the synth
     */
    JNIEXPORT void JNICALL SUBSYNTH(noteDown)(
                JNIEnv* jenv, jobject jobj, jint note, jfloat velocity);
    JNIEXPORT void JNICALL SUBSYNTH(noteUp)(
                JNIEnv* jenv, jobject jobj, jint note, jfloat velocity);
}
