#include <iostream>
#include "../src/clip_detector.h"
#include "../src/fm_synth.h"
#include "../src/limiter.h"
#include "../src/midi_wrapper.h"
#include "../src/oscillator.h"
#include "../src/speaker.h"
#include "../src/timing_manager.h"

int main()
{
    using namespace std;

    cout << "Initializing MIDI instrument" << endl;
    TimingManager timing;

    FMSynth inst(10);
    timing.add_instrument(&inst);

    MidiListener midi(timing, inst, 1);

    inst.set_modulator_intensity(5);

    cout << "Initializing signal chain" << endl;
    Limiter limiter(-3.0);
    limiter.set_input_channel(inst.get_output_channel());

    ClipDetector clip(1.0);
    clip.set_input_channel(limiter.get_output_channel());

    Speaker out(timing);
    out.set_input_channel(clip.get_output_channel());
    timing.add_consumer(&out);

    cout << "Entering playback loop..." << endl << endl;
    while(true)
        timing.tick();

    
    return 0;
}
