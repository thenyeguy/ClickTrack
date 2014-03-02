#include <jni.h>
#include "../src/elementary_filters.h"
#include "../src/io_elements.h"
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

            /* The play/pause functions will begin/stop audio output. They can
             * safely be called at any time, regardless of the current state.
             */
            void play();
            void pause();

            /* Set volume on our different components
             */
            void set_mic_gain(float gain);
            void set_osc_gain(float gain);
            void set_sub_synth_gain(float gain);

            /* Pass notes to the subtractive synth
             */
            void sub_synth_note_down(unsigned note, float velocity);
            void sub_synth_note_up(unsigned note, float velocity);

        private:
            /* Our signal chain
             */
            Microphone microphone;
            GainFilter mic_gain;

            SawWave osc;
            GainFilter osc_gain;

            SubtractiveSynth sub_synth;
            GainFilter sub_synth_gain;

            Adder master_adder;
            Speaker speaker;

            /* These
             */
            enum MasterState { PAUSED, PAUSING, PLAYING };
            MasterState state;
    };
}


/* The following bindings are wrappers for the ClickTrackMaster class. They all
 * deal with a "jlong" type. This is the pointer to the master class and cast to
 * a long so that we may return it up to java. This long is casted back to
 * a pointer within the C++ code to access our object.
 */
extern "C"
{
    /* This function will create a new ClickTrackMaster object and return
     * a pointer to it.
     */
    JNIEXPORT jlong JNICALL 
        Java_edu_cmu_ece_ece551_clicktrack_ClickTrack_initClickTrackMaster(
                JNIEnv* jenv, jobject jobj);

    /* This frees the ClickTrackMaster object passed into it
     */
    JNIEXPORT void JNICALL 
        Java_edu_cmu_ece_ece551_clicktrack_ClickTrack_freeClickTrackMaster(
                JNIEnv* jenv, jobject jobj, jlong obj);

    /* This is a wrapper for ClickTrackMaster::play(). This call spawns another
     * thread and returns after
     */
    JNIEXPORT void JNICALL 
        Java_edu_cmu_ece_ece551_clicktrack_ClickTrack_ClickTrackMasterPlay(
                JNIEnv* jenv, jobject jobj, jlong obj);

    /* This is a wrapper for ClickTrackMaster::pause()
     */
    JNIEXPORT void JNICALL 
        Java_edu_cmu_ece_ece551_clicktrack_ClickTrack_ClickTrackMasterPause(
                JNIEnv* jenv, jobject jobj, jlong obj);

    /* These set the gain on our components
     */
    JNIEXPORT void JNICALL 
        Java_edu_cmu_ece_ece551_clicktrack_ClickTrack_ClickTrackMasterSetMicGain(
                JNIEnv* jenv, jobject jobj, jlong obj, jfloat gain);
    JNIEXPORT void JNICALL 
        Java_edu_cmu_ece_ece551_clicktrack_ClickTrack_ClickTrackMasterSetOscGain(
                JNIEnv* jenv, jobject jobj, jlong obj, jfloat gain);
    JNIEXPORT void JNICALL 
        Java_edu_cmu_ece_ece551_clicktrack_ClickTrack_ClickTrackMasterSetSubSynthGain(
                JNIEnv* jenv, jobject jobj, jlong obj, jfloat gain);

    /* Play notes on the synth
     */
    JNIEXPORT void JNICALL 
        Java_edu_cmu_ece_ece551_clicktrack_ClickTrack_ClickTrackMasterSubSynthNoteDown(
                JNIEnv* jenv, jobject jobj, jlong obj, jint note, jfloat velocity);
    JNIEXPORT void JNICALL 
        Java_edu_cmu_ece_ece551_clicktrack_ClickTrack_ClickTrackMasterSubSynthNoteUp(
                JNIEnv* jenv, jobject jobj, jlong obj, jint note, jfloat velocity);
}
