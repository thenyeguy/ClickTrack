#include <chrono>
#include <thread>
#include "master_bindings.h"
#include "../src/logcat.h"

#include <time.h>

using namespace ClickTrack;

double currentTimeInMilliseconds()
{
    struct timespec res;
    clock_gettime(CLOCK_REALTIME, &res);
    return 1000.0 * res.tv_sec + (double) res.tv_nsec / 1e6;
}


ClickTrackMaster::ClickTrackMaster()
    : microphone(), mic_gain(0.0), osc(440), osc_gain(0.0),
      master_adder(2), speaker(), state(PAUSED)
      // Automatically mono
{
    // Connect the signal chain
    mic_gain.set_input_channel(microphone.get_output_channel());
    osc_gain.set_input_channel(osc.get_output_channel());

    master_adder.set_input_channel(mic_gain.get_output_channel(), 0);
    master_adder.set_input_channel(osc_gain.get_output_channel(), 1);
    speaker.set_input_channel(master_adder.get_output_channel());
}


ClickTrackMaster::~ClickTrackMaster()
{
    //Currently no work to do
}


void ClickTrackMaster::play()
{
    logi("Playing audio in native, timestamp: %f", currentTimeInMilliseconds());

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
    logi("Pausing audio in native, timestamp: %f", currentTimeInMilliseconds());

    if(state == PLAYING)
        state = PAUSING;
}


void ClickTrackMaster::set_mic_gain(float gain)
{
    logi("Setting mic gain in native to %f, timestamp: %f", gain, currentTimeInMilliseconds());

    mic_gain.set_gain(gain);
}


void ClickTrackMaster::set_osc_gain(float gain)
{
    logi("Setting osc gain in native to %f, timestamp: %f", gain, currentTimeInMilliseconds());

    osc_gain.set_gain(gain);
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


void Java_edu_cmu_ece_ece551_clicktrack_ClickTrack_ClickTrackMasterSetMicGain(
        JNIEnv* jenv, jobject jobj, jlong obj, jfloat gain)
{
    ClickTrackMaster* master = (ClickTrackMaster*) obj;
    master->set_mic_gain(gain);
}
void Java_edu_cmu_ece_ece551_clicktrack_ClickTrack_ClickTrackMasterSetOscGain(
        JNIEnv* jenv, jobject jobj, jlong obj, jfloat gain)
{
    ClickTrackMaster* master = (ClickTrackMaster*) obj;
    master->set_osc_gain(gain);
}
