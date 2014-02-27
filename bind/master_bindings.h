#include <jni.h>
#include "../src/io_elements.h"


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

        private:
            /* Our signal chain
             */
            Microphone microphone;
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
}
