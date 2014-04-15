#ifndef MASTERBINDINGS_H
#define MASTERBINDINGS_H

#include <chrono>
#include <jni.h>
#include "../src/adder.h"
#include "../src/drum_machine.h"
#include "../src/microphone.h"
#include "../src/opensles_wrapper.h"
#include "../src/oscillator.h"
#include "../src/reverb.h"
#include "../src/speaker.h"
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

            Microphone       mic;
            SubtractiveSynth sub_synth;
            DrumMachine      drum_machine;

            Adder        master_adder;
            MoorerReverb reverb;
            Speaker      speaker;

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
#define MASTER(f) Java_edu_cmu_ece_ece551_clicktrack_NativeClickTrack_##f
#define REVERB(f) Java_edu_cmu_ece_ece551_clicktrack_NativeClickTrack_00024Reverb_##f
#define SUBSYNTH(f) Java_edu_cmu_ece_ece551_clicktrack_NativeClickTrack_00024SubtractiveSynth_##f
#define DRUMMACHINE(f) Java_edu_cmu_ece_ece551_clicktrack_NativeClickTrack_00024DrumMachine_##f
extern "C"
{
/*
 * MASTER CHANNEL FEATURES
 */
    /* This function can be used to set the buffer size used for processing
     */
    JNIEXPORT void JNICALL MASTER(setBufferSize)(JNIEnv* jenv, jobject jobj, 
            jint size);

    /* These functions start and stop the audio processing. Note that unless the
     * processing is stopped, OpenSLES will maintain a wakelock
     */
    JNIEXPORT void JNICALL MASTER(start)(JNIEnv* jenv, jobject jobj);
    JNIEXPORT void JNICALL MASTER(stop)(JNIEnv* jenv, jobject jobj);

    /* This is a wrapper for ClickTrackMaster::play(). This call spawns another
     * thread and returns after
     */
    JNIEXPORT void JNICALL MASTER(play)(JNIEnv* jenv, jobject jobj);

    /* This is a wrapper for ClickTrackMaster::pause()
    */
    JNIEXPORT void JNICALL MASTER(pause)(JNIEnv* jenv, jobject jobj);


/*
 * MASTER CHANNEL REVERB
 */
    JNIEXPORT void JNICALL REVERB(setRevTime)(JNIEnv* jenv, jobject jobj,
            jfloat rev_time);
    JNIEXPORT void JNICALL REVERB(setGain)(JNIEnv* jenv, jobject jobj,
            jfloat gain);
    JNIEXPORT void JNICALL REVERB(setWetness)(JNIEnv* jenv, jobject jobj,
            jfloat wetness);


/* 
 * SUBTRACTIVE SYNTH FEATURES
 */
    /* First set note events
    */
    JNIEXPORT void JNICALL SUBSYNTH(noteDown)(JNIEnv* jenv, jobject jobj,
            jint note, jfloat velocity);
    JNIEXPORT void JNICALL SUBSYNTH(noteUp)(JNIEnv* jenv, jobject jobj,
            jint note, jfloat velocity);

    /* Oscillator modes. 
     * Mode will be an integer constant defined in Java to match the enum
     */
    JNIEXPORT void JNICALL SUBSYNTH(setOsc1Mode)(JNIEnv* jenv, jobject jobj,
            jint mode);
    JNIEXPORT void JNICALL SUBSYNTH(setOsc2Mode)(JNIEnv* jenv, jobject jobj,
            jint mode);

    /* Oscillator transpositions. 
     */
    JNIEXPORT void JNICALL SUBSYNTH(setOsc1Transposition)(JNIEnv* jenv, 
            jobject jobj, jfloat steps);
    JNIEXPORT void JNICALL SUBSYNTH(setOsc2Transposition)(JNIEnv* jenv, 
            jobject jobj, jfloat steps);

    /* ADSR Envelope
    */
    JNIEXPORT void JNICALL SUBSYNTH(setAttackTime)(JNIEnv* jenv, jobject jobj,
            jfloat attack_time);
    JNIEXPORT void JNICALL SUBSYNTH(setDecayTime)(JNIEnv* jenv, jobject jobj,
            jfloat decay_time);
    JNIEXPORT void JNICALL SUBSYNTH(setSustainLevel)(JNIEnv* jenv, jobject jobj,
            jfloat sustain_level);
    JNIEXPORT void JNICALL SUBSYNTH(setReleaseTime)(JNIEnv* jenv, jobject jobj,
            jfloat release_time);

    /* Filter controls
     * Mode will be an integer constant defined in Java to match the enum
     */
    JNIEXPORT void JNICALL SUBSYNTH(setFilterMode)(JNIEnv* jenv, jobject jobj,
            jint mode);
    JNIEXPORT void JNICALL SUBSYNTH(setFilterCutoff)(JNIEnv* jenv, jobject jobj,
            jfloat cutoff);
    JNIEXPORT void JNICALL SUBSYNTH(setFilterGain)(JNIEnv* jenv, jobject jobj,
            jfloat gain);
    JNIEXPORT void JNICALL SUBSYNTH(setFilterQ)(JNIEnv* jenv, jobject jobj,
            jfloat q);

    /* LFO settings
     */
    JNIEXPORT void JNICALL SUBSYNTH(setLfoMode)(JNIEnv* jenv, jobject jobj,
            jint mode);
    JNIEXPORT void JNICALL SUBSYNTH(setLfoFreq)(JNIEnv* jenv, jobject jobj,
            jfloat freq);
    JNIEXPORT void JNICALL SUBSYNTH(setLfoVibrato)(JNIEnv* jenv, jobject jobj,
            jfloat steps);
    JNIEXPORT void JNICALL SUBSYNTH(setLfoTremelo)(JNIEnv* jenv, jobject jobj,
            jfloat db);


    /* Final output gain
    */
    JNIEXPORT void JNICALL SUBSYNTH(setGain)(JNIEnv* jenv, jobject jobj,
            jfloat gain);

/* 
 * DRUM MACHINE FEATURES
 */
    /* First set note events
    */
    JNIEXPORT void JNICALL DRUMMACHINE(noteDown)(JNIEnv* jenv, jobject jobj,
            jint note, jfloat velocity);

    /* Setters
    */
    JNIEXPORT void JNICALL DRUMMACHINE(setGain)(JNIEnv* jenv, jobject jobj,
            jfloat gain);
    JNIEXPORT void JNICALL DRUMMACHINE(setVoice)(JNIEnv* jenv, jobject jobj,
            jstring path);
}

#endif
