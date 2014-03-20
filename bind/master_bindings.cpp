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
      sub_synth(10), drum_machine(""), master_adder(2), 
      reverb(MoorerReverb::HALL, 1.0, 1.0, 0.0, 1), speaker()
      // Automatically mono
{
    // Connect the signal chain
    master_adder.set_input_channel(sub_synth.get_output_channel(), 0);
    master_adder.set_input_channel(drum_machine.get_output_channel(), 1);

    reverb.set_input_channel(master_adder.get_output_channel());
    speaker.set_input_channel(reverb.get_output_channel());

    // Register this as the callback for speakers
    speaker.register_callback(ClickTrackMaster::timing_callback, this);
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
        speaker.consume();
    state = PAUSED;
}


void ClickTrackMaster::pause()
{
    if(state == PLAYING)
        state = PAUSING;
}


void ClickTrackMaster::timing_callback(unsigned long time, void* payload)
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
        time = next_buffer_time + BUFFER_SIZE + delay;
    }
    return time;
}




void MASTER(play)(JNIEnv* jenv, jobject jobj)
{
    ClickTrackMaster& master = ClickTrackMaster::get_instance();
    std::thread player(&ClickTrackMaster::play, &master);
    player.detach();
}


void MASTER(pause)(JNIEnv* jenv, jobject jobj)
{
    ClickTrackMaster& master = ClickTrackMaster::get_instance();
    master.pause();
}


void MASTER(start)(JNIEnv* jenv, jobject jobj)
{
    ClickTrackMaster& master = ClickTrackMaster::get_instance();
    master.start();
}


void MASTER(stop)(JNIEnv* jenv, jobject jobj)
{
    ClickTrackMaster& master = ClickTrackMaster::get_instance();
    master.stop();
}




void REVERB(setRevTime)(JNIEnv* jenv, jobject jobj, jfloat rev_time)
{
    ClickTrackMaster& master = ClickTrackMaster::get_instance();
    master.reverb.set_rev_time(rev_time);
}
void REVERB(setGain)(JNIEnv* jenv, jobject jobj, jfloat gain)
{
    ClickTrackMaster& master = ClickTrackMaster::get_instance();
    master.reverb.set_gain(gain);
}
void REVERB(setWetness)(JNIEnv* jenv, jobject jobj, jfloat wetness)
{
    ClickTrackMaster& master = ClickTrackMaster::get_instance();
    master.reverb.set_wetness(wetness);
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


void SUBSYNTH(setOsc1Mode)(JNIEnv* jenv, jobject jobj, jint mode)
{
    ClickTrackMaster& master = ClickTrackMaster::get_instance();
    master.sub_synth.set_osc1_mode((Oscillator::OscMode) mode);
}


void SUBSYNTH(setOsc2Mode)(JNIEnv* jenv, jobject jobj, jint mode)
{
    ClickTrackMaster& master = ClickTrackMaster::get_instance();
    master.sub_synth.set_osc2_mode((Oscillator::OscMode) mode);
}


void SUBSYNTH(set_attack_time)(JNIEnv* jenv, jobject jobj, jfloat attack_time)
{
    ClickTrackMaster& master = ClickTrackMaster::get_instance();
    master.sub_synth.set_attack_time(attack_time);
}


void SUBSYNTH(set_decay_time)(JNIEnv* jenv, jobject jobj, jfloat decay_time)
{
    ClickTrackMaster& master = ClickTrackMaster::get_instance();
    master.sub_synth.set_decay_time(decay_time);
}


void SUBSYNTH(set_sustain_level)(JNIEnv* jenv, jobject jobj, jfloat
        sustain_level)
{
    ClickTrackMaster& master = ClickTrackMaster::get_instance();
    master.sub_synth.set_sustain_level(sustain_level);
}


void SUBSYNTH(set_release_time)(JNIEnv* jenv, jobject jobj, jfloat
        release_time)
{
    ClickTrackMaster& master = ClickTrackMaster::get_instance();
    master.sub_synth.set_release_time(release_time);
}


void SUBSYNTH(setPoint1)( JNIEnv* jenv, jobject jobj, jint mode, jfloat cutoff,
        jfloat gain)
{
    ClickTrackMaster& master = ClickTrackMaster::get_instance();
    master.sub_synth.eq.setPoint1((FourPointEqualizer::EQFilterMode) mode,
            cutoff, gain);
}


void SUBSYNTH(setPoint4)( JNIEnv* jenv, jobject jobj, jint mode, jfloat cutoff,
        jfloat gain)
{
    ClickTrackMaster& master = ClickTrackMaster::get_instance();
    master.sub_synth.eq.setPoint4((FourPointEqualizer::EQFilterMode) mode,
            cutoff, gain);
}


void SUBSYNTH(setPoint2)( JNIEnv* jenv, jobject jobj, jfloat cutoff, jfloat Q,
        jfloat gain)
{
    ClickTrackMaster& master = ClickTrackMaster::get_instance();
    master.sub_synth.eq.setPoint2(cutoff, Q, gain);
}


void SUBSYNTH(setPoint3)( JNIEnv* jenv, jobject jobj, jfloat cutoff, jfloat Q,
        jfloat gain)
{
    ClickTrackMaster& master = ClickTrackMaster::get_instance();
    master.sub_synth.eq.setPoint3(cutoff, Q, gain);
}


void SUBSYNTH(setGain)(JNIEnv* jenv, jobject jobj, 
        jfloat gain)
{
    ClickTrackMaster& master = ClickTrackMaster::get_instance();
    master.sub_synth.volume.set_gain(gain);
}


void DRUMMACHINE(noteDown)(JNIEnv* jenv, jobject jobj, jint note, 
        jfloat velocity)
{
    ClickTrackMaster& master = ClickTrackMaster::get_instance();
    unsigned long time = master.get_timestamp();
    master.drum_machine.on_note_down(note, velocity, time);
}


void DRUMMACHINE(setVoice)(JNIEnv* jenv, jobject jobj, jstring jpath)
{
    // Get the path from the jstring
    const char *s = jenv->GetStringUTFChars(jpath,NULL);
    std::string path(s);

    // Set the new voice
    ClickTrackMaster& master = ClickTrackMaster::get_instance();
    master.drum_machine.set_voice(path);

    // Release the string
    jenv->ReleaseStringUTFChars(jpath,s);
}
