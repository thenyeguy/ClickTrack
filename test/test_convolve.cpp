#include <iostream>
#include "../src/convolution_reverb.h"
#include "../src/speaker.h"
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
        WavReader in("wav/test.wav");

        ConvolutionReverb revl(imp->num_samples, imp->left, 0.1, 0.5);
        revl.set_input_channel(in.get_output_channel(0));
        ConvolutionReverb revr(imp->num_samples, imp->right, 0.1, 0.5);
        revr.set_input_channel(in.get_output_channel(1));


        WavWriter out("wav/conv_out.wav");
        out.set_input_channel(revl.get_output_channel(),0);
        out.set_input_channel(revr.get_output_channel(),1);

        Speaker speaker(2);
        speaker.set_input_channel(revl.get_output_channel(),0);
        speaker.set_input_channel(revr.get_output_channel(),1);

        std::cout << "Playing" << std::endl;
        while(!in.is_done())
        {
            out.consume();
            speaker.consume();
        }
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
