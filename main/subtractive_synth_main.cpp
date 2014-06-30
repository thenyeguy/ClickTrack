#include <iostream>
#include "../src/clip_detector.h"
#include "../src/limiter.h"
#include "../src/midi_listener.h"
#include "../src/ring_modulator.h"
#include "../src/speaker.h"
#include "../src/subtractive_synth.h"
#include "../src/timing_manager.h"

int main()
{
    using namespace std;

    cout << "Initializing MIDI instrument" << endl;
    TimingManager timing;

    MidiListener midi(timing, 1);
    SubtractiveSynth inst(10);
    inst.set_input_midi_channel(midi.get_output_midi_channel());
    timing.add_midi_consumer(&inst);

    inst.filter.set_cutoff(1000);
    inst.set_lfo_vibrato(0.1);
    inst.set_lfo_tremolo(.2);

    cout << "Initializing signal chain" << endl;

    Limiter limiter(-3.0);
    limiter.set_input_channel(inst.get_output_channel());

    Speaker out(timing);
    out.set_input_channel(limiter.get_output_channel());
    timing.add_audio_consumer(&out);

    cout << "Entering playback loop..." << endl << endl;
    while(true)
        timing.tick();
    
    return 0;
}
