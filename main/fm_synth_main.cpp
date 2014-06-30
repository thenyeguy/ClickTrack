#include <iostream>
#include "../src/clip_detector.h"
#include "../src/fm_synth.h"
#include "../src/limiter.h"
#include "../src/midi_listener.h"
#include "../src/oscillator.h"
#include "../src/speaker.h"
#include "../src/timing_manager.h"

int main()
{
    using namespace std;

    cout << "Initializing MIDI instrument" << endl;
    TimingManager timing;

    MidiListener midi(timing, 1);
    FMSynth inst(10);
    inst.set_input_midi_channel(midi.get_output_midi_channel());
    timing.add_midi_consumer(&inst);


    inst.set_modulator_intensity(5);

    cout << "Initializing signal chain" << endl;
    Limiter limiter(-3.0);
    limiter.set_input_channel(inst.get_output_channel());

    ClipDetector clip(1.0);
    clip.set_input_channel(limiter.get_output_channel());

    Speaker out(timing);
    out.set_input_channel(clip.get_output_channel());
    timing.add_audio_consumer(&out);

    cout << "Entering playback loop..." << endl << endl;
    while(true)
        timing.tick();

    
    return 0;
}
