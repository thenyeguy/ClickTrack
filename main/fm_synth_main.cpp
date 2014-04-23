#include <iostream>
#include "../src/clip_detector.h"
#include "../src/limiter.h"
#include "../src/midi_wrapper.h"
#include "../src/oscillator.h"
#include "../src/speaker.h"
#include "../src/subtractive_synth.h"

int main()
{
    using namespace std;

    cout << "Initializing MIDI instrument" << endl;
    // SubtractiveSynth inst(10);
    // MidiListener midi(&inst, 1);

    Oscillator carrier(Oscillator::Sine, 440);
    Oscillator mod(Oscillator::Saw, 440);
    
    carrier.set_modulator_input(mod.get_output_channel());
    carrier.set_modulator_intensity(20);

    cout << "Initializing signal chain" << endl;
    Limiter limiter(-3.0);
    limiter.set_input_channel(carrier.get_output_channel());

    ClipDetector clip(1.0);
    clip.set_input_channel(limiter.get_output_channel());

    Speaker out;
    out.set_input_channel(clip.get_output_channel());
    // out.register_callback(MidiListener::timing_callback, &midi);

    cout << "Entering playback loop..." << endl << endl;
    while(true)
    {
        out.consume();
    }

    
    return 0;
}
