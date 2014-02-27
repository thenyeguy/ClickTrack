#include <chrono>
#include <thread>
#include "master_bindings.h"

using namespace ClickTrack;


ClickTrackMaster::ClickTrackMaster()
    : microphone(), speaker(), state(PAUSED)
      // Automatically mono
{
    // Connect the signal chain
    speaker.set_input_channel(microphone.get_output_channel());
}


ClickTrackMaster::~ClickTrackMaster()
{
    //Currently no work to do
}


void ClickTrackMaster::play()
{
    // If we are already playing in another thread, don't start another loop.
    if(state == PLAYING) 
        return;

    // If we are stopping, wait for it to finish before starting again
    while(state == PAUSING)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    // Loop until ended by an outside pause call
    state = PLAYING;
    while(state == PLAYING)
        speaker.consume_inputs();
    state = PAUSED;
}


void ClickTrackMaster::pause()
{
    if(state == PLAYING)
        state = PAUSING;
}




jlong Java_edu_cmu_ece_ece551_clicktrack_ClickTrack_initClickTrackMaster(
        JNIEnv* jenv, jobject jobj)
{
    ClickTrackMaster* obj = new ClickTrackMaster();
    return (jlong) obj;
}

void Java_edu_cmu_ece_ece551_clicktrack_ClickTrack_freeClickTrackMaster(
        JNIEnv* jenv, jobject jobj, jlong obj)
{
    ClickTrackMaster* master = (ClickTrackMaster*) obj;
    master->pause();
    delete master;
}

void Java_edu_cmu_ece_ece551_clicktrack_ClickTrack_ClickTrackMasterPlay(
        JNIEnv* jenv, jobject jobj, jlong obj)
{
    ClickTrackMaster* master = (ClickTrackMaster*) obj;
    std::thread player(&ClickTrackMaster::play, master);
    player.detach();
}

void Java_edu_cmu_ece_ece551_clicktrack_ClickTrack_ClickTrackMasterPause(
        JNIEnv* jenv, jobject jobj, jlong obj)
{
    ClickTrackMaster* master = (ClickTrackMaster*) obj;
    master->pause();
}
