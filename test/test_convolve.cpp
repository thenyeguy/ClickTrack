#include <iostream>
#include "../src/convolution_filter.h"
#include "../src/speaker.h"
#include "../src/timing_manager.h"
#include "../src/wav_reader.h"
#include "../src/wav_writer.h"

using namespace ClickTrack;


int main()
{
    try
    {
        std::cout << "Reading in impulse" << std::endl;
        impulse_pair* imp = impulse_from_wav("wav/test_impulse.wav");

        std::cout << "Establishing signal chain" << std::endl;
        TimingManager timing;

        WavReader in("wav/test.wav");

        ConvolutionFilter revl(imp->num_samples, imp->left, 0.1, 0.5);
        revl.set_input_channel(in.get_output_channel(0));
        ConvolutionFilter revr(imp->num_samples, imp->right, 0.1, 0.5);
        revr.set_input_channel(in.get_output_channel(1));


        WavWriter out("wav/conv_out.wav");
        out.set_input_channel(revl.get_output_channel(),0);
        out.set_input_channel(revr.get_output_channel(),1);
        timing.add_audio_consumer(&out);

        Speaker speaker(timing, 2);
        speaker.set_input_channel(revl.get_output_channel(),0);
        speaker.set_input_channel(revr.get_output_channel(),1);
        timing.add_audio_consumer(&speaker);

        std::cout << "Entering playback loop..." << std::endl << std::endl;
        while(!in.is_done())
            timing.tick();
        std::cout << "Done" << std::endl;
    }
    catch(std::exception& e)
    {
        std::cerr << "\n\n" << "EXCEPTION: " << typeid(e).name() << std::endl;
        std::cerr << "           " << e.what() << std::endl;

        exit(1);
    }

    return 0;
}
