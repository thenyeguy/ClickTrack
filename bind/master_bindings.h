#include <jni.h>
#include "../src/elementary_filters.h"
#include "../src/io_elements.h"
#include "../src/opensles_wrapper.h"
#include "../src/oscillator.h"
#include "../src/subtractive_synth.h"


namespace ClickTrack
{
    /* This class is a master container for the ClickTrack processing backend in
     * C++. It is responsible for containing all the global state in the
     * native code end.
     */
    class ClickTrackMaster
    {
        public:
            /* Constructor/destructor handles our entire audio chain
             */
            ClickTrackMaster();
            ~ClickTrackMaster();

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
    };
}


/* The following bindings are wrappers for the ClickTrackMaster class. They all
 * deal with a "jlong" type. This is the pointer to the master class and cast to
 * a long so that we may return it up to java. This long is casted back to
 * a pointer within the C++ code to access our object.
 */
// Concatenate the package name on our functions
#define PACKAGE(f) Java_edu_cmu_ece_ece551_clicktrack_ClickTrack_native##f
extern "C"
{
    /* This function will create a new ClickTrackMaster object and return
     * a pointer to it.
     */
    JNIEXPORT jlong JNICALL PACKAGE(InitClickTrackMaster)(
                JNIEnv* jenv, jobject jobj);

    /* This frees the ClickTrackMaster object passed into it
     */
    JNIEXPORT void JNICALL PACKAGE(FreeClickTrackMaster)(
                JNIEnv* jenv, jobject jobj, jlong obj);

    /* These functions start and stop the audio processing. Note that unless the
     * processing is stopped, OpenSLES will maintain a wakelock
     */
    JNIEXPORT void JNICALL PACKAGE(Start)(
                JNIEnv* jenv, jobject jobj, jlong obj);
    JNIEXPORT void JNICALL PACKAGE(Stop)(
                JNIEnv* jenv, jobject jobj, jlong obj);

    /* This is a wrapper for ClickTrackMaster::play(). This call spawns another
     * thread and returns after
     */
    JNIEXPORT void JNICALL PACKAGE(Play)(
                JNIEnv* jenv, jobject jobj, jlong obj);

    /* This is a wrapper for ClickTrackMaster::pause()
     */
    JNIEXPORT void JNICALL PACKAGE(Pause)(
                JNIEnv* jenv, jobject jobj, jlong obj);

    /* These set the gain on our components
     */
    JNIEXPORT void JNICALL PACKAGE(MicSetGain)(
                JNIEnv* jenv, jobject jobj, jlong obj, jfloat gain);
    JNIEXPORT void JNICALL PACKAGE(OscSetGain)(
                JNIEnv* jenv, jobject jobj, jlong obj, jfloat gain);
    JNIEXPORT void JNICALL PACKAGE(SubtractiveSynthSetGain)(
                JNIEnv* jenv, jobject jobj, jlong obj, jfloat gain);

    /* Play notes on the synth
     */
    JNIEXPORT void JNICALL PACKAGE(SubtractiveSynthNoteDown)(
                JNIEnv* jenv, jobject jobj, jlong obj, jint note, jfloat velocity);
    JNIEXPORT void JNICALL PACKAGE(SubtractiveSynthNoteUp)(
                JNIEnv* jenv, jobject jobj, jlong obj, jint note, jfloat velocity);
}
