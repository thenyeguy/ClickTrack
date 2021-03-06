#include <cstdlib>
#include <iostream>
#include "../src/gain_filter.h"
#include "../src/oscillator.h"
#include "../src/speaker.h"
#include "../src/timing_manager.h"
#include "../src/wav_writer.h"

using namespace ClickTrack;


int main()
{
    std::cout << "Initializing signal chain" << std::endl;
    TimingManager timer;

    // Set up oscillator
    Oscillator osc(Oscillator::BlepTri, 440);
    osc.set_transposition(-3);

    // Set up LFO
    Oscillator lfo(Oscillator::Sine, 5);
    osc.set_lfo_input(lfo.get_output_channel());
    osc.set_lfo_intensity(.3);

    GainFilter gain(0.0);
    gain.set_input_channel(osc.get_output_channel());
    gain.set_lfo_input(lfo.get_output_channel());
    gain.set_lfo_intensity(3);

    Speaker speaker(timer);
    speaker.set_input_channel(gain.get_output_channel());
    timer.add_audio_consumer(&speaker);


    // Loop small number of times to generate enough audio
    std::cout << "Entering process loop" << std::endl;
    while(true)
    {
        try
        {
            timer.tick();
        }
        catch(std::exception& e)
        {
            std::cerr << "\n\n" << "EXCEPTION: " << e.what() << std::endl;
            exit(1);
        }
    }
}
