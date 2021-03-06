#include <cstdlib>
#include <iostream>
#include "../src/compressor.h"
#include "../src/limiter.h"
#include "../src/noise_gate.h"
#include "../src/timing_manager.h"
#include "../src/wav_reader.h"
#include "../src/wav_writer.h"

using namespace ClickTrack;


int main()
{
    std::cout << "Initializing signal chain" << std::endl;
    TimingManager timer;

    WavReader test_wav("wav/volume_up.wav");

    Limiter limiter(-6);
    limiter.set_input_channel(test_wav.get_output_channel());
    WavWriter limiter_wav("wav/test_limiter.wav");
    limiter_wav.set_input_channel(limiter.get_output_channel());
    timer.add_audio_consumer(&limiter_wav);

    Compressor compressor(-6, 0.5);
    compressor.set_input_channel(test_wav.get_output_channel());
    WavWriter compressor_wav("wav/test_compressor.wav");
    compressor_wav.set_input_channel(compressor.get_output_channel());
    timer.add_audio_consumer(&compressor_wav);

    NoiseGate noise_gate(-6, -9);
    noise_gate.set_input_channel(test_wav.get_output_channel());
    WavWriter noise_gate_wav("wav/test_noise_gate.wav");
    noise_gate_wav.set_input_channel(noise_gate.get_output_channel());
    timer.add_audio_consumer(&noise_gate_wav);

    std::cout << "Entering process loop" << std::endl;
    for(unsigned i = 0; i < test_wav.get_total_samples(); i++)
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
    std::cout << "Complete" << std::endl;
}
