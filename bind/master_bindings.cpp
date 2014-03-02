#include <chrono>
#include <thread>
#include "master_bindings.h"
#include "../src/logcat.h"

#include <time.h>

using namespace ClickTrack;


ClickTrackMaster::ClickTrackMaster()
    : microphone(), mic_gain(0.0), osc(440), osc_gain(0.0), sub_synth(10), 
      sub_synth_gain(0.0), master_adder(3), speaker(), state(PAUSED)
      // Automatically mono
{
    // Connect the signal chain
    mic_gain.set_input_channel(microphone.get_output_channel());
    osc_gain.set_input_channel(osc.get_output_channel());
    sub_synth_gain.set_input_channel(sub_synth.get_output_channel());

    master_adder.set_input_channel(mic_gain.get_output_channel(), 0);
    master_adder.set_input_channel(osc_gain.get_output_channel(), 1);
    master_adder.set_input_channel(sub_synth_gain.get_output_channel(), 2);
    speaker.set_input_channel(master_adder.get_output_channel());
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


void ClickTrackMaster::set_mic_gain(float gain)
{
    mic_gain.set_gain(gain);
}


void ClickTrackMaster::set_osc_gain(float gain)
{
    osc_gain.set_gain(gain);
}


void ClickTrackMaster::set_sub_synth_gain(float gain)
{
    sub_synth_gain.set_gain(gain);
}


void ClickTrackMaster::sub_synth_note_down(unsigned note, float velocity)
{
    sub_synth.on_note_down(note, velocity);
}


void ClickTrackMaster::sub_synth_note_up(unsigned note, float velocity)
{
    sub_synth.on_note_up(note, velocity);
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
void Java_edu_cmu_ece_ece551_clicktrack_ClickTrack_ClickTrackMasterSetSubSynthGain(
        JNIEnv* jenv, jobject jobj, jlong obj, jfloat gain)
{
    ClickTrackMaster* master = (ClickTrackMaster*) obj;
    master->set_sub_synth_gain(gain);
}


void Java_edu_cmu_ece_ece551_clicktrack_ClickTrack_ClickTrackMasterSubSynthNoteDown(
        JNIEnv* jenv, jobject jobj, jlong obj, jint note, jfloat velocity)
{
    ClickTrackMaster* master = (ClickTrackMaster*) obj;
    master->sub_synth_note_down(note, velocity);
}
void Java_edu_cmu_ece_ece551_clicktrack_ClickTrack_ClickTrackMasterSubSynthNoteUp(
        JNIEnv* jenv, jobject jobj, jlong obj, jint note, jfloat velocity)
{
    ClickTrackMaster* master = (ClickTrackMaster*) obj;
    master->sub_synth_note_down(note, velocity);
}
