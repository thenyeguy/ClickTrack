#include <thread>
#include "master_bindings.h"
#include "../src/logcat.h"

using namespace ClickTrack;
namespace chr = std::chrono;


ClickTrackMaster& ClickTrackMaster::get_instance()
{
    static ClickTrackMaster instance;
    return instance;
}


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

    // Register this as the callback for speakers
    speaker.register_callback(ClickTrackMaster::consumer_callback, this);
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
    // Stop playback and wait for it to finish
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


void ClickTrackMaster::consumer_callback(unsigned long time, void* payload)
{
    // Store the current time
    ClickTrackMaster* master = (ClickTrackMaster*) payload;
    master->buffer_timestamp = chr::high_resolution_clock::now();
    master->next_buffer_time = time;
}

unsigned long ClickTrackMaster::get_timestamp()
{
    unsigned long time = 0;
    if(next_buffer_time != 0) 
    {
        // Compute the time difference
        auto diff = chr::high_resolution_clock::now() - buffer_timestamp;
        double nanos = chr::duration_cast<chr::nanoseconds>(diff).count();

        // Convert to a sample delay
        unsigned long delay = nanos / 1e9 * SAMPLE_RATE;

        // Add this delay, one buffer behind
        time = next_buffer_time + FRAME_SIZE + delay;
    }
    return time;
}




void PACKAGE(play)(JNIEnv* jenv, jobject jobj)
{
    ClickTrackMaster& master = ClickTrackMaster::get_instance();
    std::thread player(&ClickTrackMaster::play, &master);
    player.detach();
}

void PACKAGE(pause)(JNIEnv* jenv, jobject jobj)
{
    ClickTrackMaster& master = ClickTrackMaster::get_instance();
    master.pause();
}

void PACKAGE(start)(JNIEnv* jenv, jobject jobj)
{
    ClickTrackMaster& master = ClickTrackMaster::get_instance();
    master.start();
}

void PACKAGE(stop)(JNIEnv* jenv, jobject jobj)
{
    ClickTrackMaster& master = ClickTrackMaster::get_instance();
    master.stop();
}


void PACKAGE(setMicGain)(JNIEnv* jenv, jobject jobj, 
        jfloat gain)
{
    ClickTrackMaster& master = ClickTrackMaster::get_instance();
    master.mic_gain.set_gain(gain);
}

void PACKAGE(setOscGain)(JNIEnv* jenv, jobject jobj, 
        jfloat gain)
{
    ClickTrackMaster& master = ClickTrackMaster::get_instance();
    master.osc_gain.set_gain(gain);


}
void SUBSYNTH(setGain)(JNIEnv* jenv, jobject jobj, 
        jfloat gain)
{
    ClickTrackMaster& master = ClickTrackMaster::get_instance();
    master.sub_synth_gain.set_gain(gain);
}


void SUBSYNTH(noteDown)(JNIEnv* jenv, jobject jobj, 
        jint note, jfloat velocity)
{
    ClickTrackMaster& master = ClickTrackMaster::get_instance();
    unsigned long time = master.get_timestamp();
    master.sub_synth.on_note_down(note, velocity, time);
}
void SUBSYNTH(noteUp)(JNIEnv* jenv, jobject jobj, 
        jint note, jfloat velocity)
{
    ClickTrackMaster& master = ClickTrackMaster::get_instance();
    unsigned long time = master.get_timestamp();
    master.sub_synth.on_note_up(note, velocity, time);
}
