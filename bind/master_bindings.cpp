#include <chrono>
#include <thread>
#include "master_bindings.h"
#include "../src/logcat.h"

#include <time.h>

using namespace ClickTrack;


ClickTrackMaster::ClickTrackMaster()
    : state(PAUSED), openSles(OpenSlesWrapper::get_instance()),
      microphone(), mic_gain(0.0), osc(440), osc_gain(0.0), sub_synth(10), 
      sub_synth_gain(0.0), master_adder(3), speaker()
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


void ClickTrackMaster::start()
{
    // Start the backend
    openSles.start();
}


void ClickTrackMaster::stop()
{
    // Stop playback and wiat for it to finish
    pause();
    while(state != PAUSED)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    // Stop the backend
    openSles.stop();
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




jlong PACKAGE(InitClickTrackMaster)(JNIEnv* jenv, jobject jobj)
{
    ClickTrackMaster* obj = new ClickTrackMaster();
    return (jlong) obj;
}

void PACKAGE(FreeClickTrackMaster)(JNIEnv* jenv, jobject jobj, jlong obj)
{
    ClickTrackMaster* master = (ClickTrackMaster*) obj;
    master->pause();
    delete master;
}

void PACKAGE(Play)(JNIEnv* jenv, jobject jobj, jlong obj)
{
    ClickTrackMaster* master = (ClickTrackMaster*) obj;
    std::thread player(&ClickTrackMaster::play, master);
    player.detach();
}

void PACKAGE(Pause)(JNIEnv* jenv, jobject jobj, jlong obj)
{
    ClickTrackMaster* master = (ClickTrackMaster*) obj;
    master->pause();
}

void PACKAGE(Start)(JNIEnv* jenv, jobject jobj, jlong obj)
{
    ClickTrackMaster* master = (ClickTrackMaster*) obj;
    master->start();
}

void PACKAGE(Stop)(JNIEnv* jenv, jobject jobj, jlong obj)
{
    ClickTrackMaster* master = (ClickTrackMaster*) obj;
    master->stop();
}


void PACKAGE(MicSetGain)(JNIEnv* jenv, jobject jobj, 
        jlong obj, jfloat gain)
{
    ClickTrackMaster* master = (ClickTrackMaster*) obj;
    master->mic_gain.set_gain(gain);
}

void PACKAGE(OscSetGain)(JNIEnv* jenv, jobject jobj, 
        jlong obj, jfloat gain)
{
    ClickTrackMaster* master = (ClickTrackMaster*) obj;
    master->osc_gain.set_gain(gain);


}
void PACKAGE(SubtractiveSynthSetGain)(JNIEnv* jenv, jobject jobj, 
        jlong obj, jfloat gain)
{
    ClickTrackMaster* master = (ClickTrackMaster*) obj;
    master->sub_synth_gain.set_gain(gain);
}


void PACKAGE(SubtractiveSynthNoteDown)(JNIEnv* jenv, jobject jobj, 
        jlong obj, jint note, jfloat velocity)
{
    ClickTrackMaster* master = (ClickTrackMaster*) obj;
    master->sub_synth.on_note_down(note, velocity);
}
void PACKAGE(SubtractiveSynthNoteUp)(JNIEnv* jenv, jobject jobj, 
        jlong obj, jint note, jfloat velocity)
{
    ClickTrackMaster* master = (ClickTrackMaster*) obj;
    master->sub_synth.on_note_up(note, velocity);
}
