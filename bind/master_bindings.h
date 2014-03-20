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
            static void timing_callback(unsigned long time, void* payload);
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

            Oscillator osc;
            GainFilter osc_gain;

            SubtractiveSynth sub_synth;

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
#define TOP(f) Java_edu_cmu_ece_ece551_clicktrack_ClickTrack_##f
#define SUBSYNTH(f) Java_edu_cmu_ece_ece551_clicktrack_ClickTrack_00024SubtractiveSynth_##f
extern "C"
{
/*
 * TOP LEVEL FEATURES
 */
    /* These functions start and stop the audio processing. Note that unless the
     * processing is stopped, OpenSLES will maintain a wakelock
     */
    JNIEXPORT void JNICALL TOP(start)(
                JNIEnv* jenv, jobject jobj);
    JNIEXPORT void JNICALL TOP(stop)(
                JNIEnv* jenv, jobject jobj);

    /* This is a wrapper for ClickTrackMaster::play(). This call spawns another
     * thread and returns after
     */
    JNIEXPORT void JNICALL TOP(play)(
                JNIEnv* jenv, jobject jobj);

    /* This is a wrapper for ClickTrackMaster::pause()
     */
    JNIEXPORT void JNICALL TOP(pause)(
                JNIEnv* jenv, jobject jobj);

    /* These set the gain on our generic components
     */
    JNIEXPORT void JNICALL TOP(setMicGain)(
                JNIEnv* jenv, jobject jobj, jfloat gain);
    JNIEXPORT void JNICALL TOP(setOscGain)(
                JNIEnv* jenv, jobject jobj, jfloat gain);


/* 
 * SUBTRACTIVE SYNTH FEATURES
 */
     /* First set note events
     */
    JNIEXPORT void JNICALL SUBSYNTH(noteDown)(
                JNIEnv* jenv, jobject jobj, jint note, jfloat velocity);
    JNIEXPORT void JNICALL SUBSYNTH(noteUp)(
                JNIEnv* jenv, jobject jobj, jint note, jfloat velocity);

    /* Oscillator modes. 
     * Mode will be an integer constant defined in Java to match the enum
     */
    JNIEXPORT void JNICALL SUBSYNTH(setOsc1Mode)(
                JNIEnv* jenv, jobject jobj, jint mode);
    JNIEXPORT void JNICALL SUBSYNTH(setOsc2Mode)(
                JNIEnv* jenv, jobject jobj, jint mode);

    /* ADSR Envelope
     */
    JNIEXPORT void JNICALL SUBSYNTH(set_attack_time)(
                JNIEnv* jenv, jobject jobj, jfloat attack_time);
    JNIEXPORT void JNICALL SUBSYNTH(set_decay_time)(
                JNIEnv* jenv, jobject jobj, jfloat decay_time);
    JNIEXPORT void JNICALL SUBSYNTH(set_sustain_level)(
                JNIEnv* jenv, jobject jobj, jfloat sustain_level);
    JNIEXPORT void JNICALL SUBSYNTH(set_release_time)(
                JNIEnv* jenv, jobject jobj, jfloat release_time);

    /* Equalizer. Filter mode is a set of integer constants in Java
     */
    JNIEXPORT void JNICALL SUBSYNTH(setPoint1)(
                JNIEnv* jenv, jobject jobj, jint mode, jfloat cutoff, 
                jfloat gain);
    JNIEXPORT void JNICALL SUBSYNTH(setPoint4)(
                JNIEnv* jenv, jobject jobj, jint mode, jfloat cutoff, 
                jfloat gain);

    JNIEXPORT void JNICALL SUBSYNTH(setPoint2)(
                JNIEnv* jenv, jobject jobj, jfloat cutoff, jfloat Q, 
                jfloat gain);
    JNIEXPORT void JNICALL SUBSYNTH(setPoint3)(
                JNIEnv* jenv, jobject jobj, jfloat cutoff, jfloat Q, 
                jfloat gain);

    /* Final output gain
     */
    JNIEXPORT void JNICALL SUBSYNTH(setGain)(
                JNIEnv* jenv, jobject jobj, jfloat gain);
}
