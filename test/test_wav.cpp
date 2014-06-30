#include <iostream>
#include "../src/speaker.h"
#include "../src/timing_manager.h"
#include "../src/wav_reader.h"
#include "../src/wav_writer.h"

using namespace ClickTrack;


int main()
{
    try
    {
        std::cout << "Initializing signal chain" << std::endl;
        WavReader wav("wav/test.wav");

        Speaker speaker;
        speaker.set_input_channel(wav.get_output_channel(0),0);
        speaker.set_input_channel(wav.get_output_channel(1),1);

        WavWriter write("wav/out.wav");
        write.set_input_channel(wav.get_output_channel(0),0);
        write.set_input_channel(wav.get_output_channel(1),1);

        TimingManager timer;
        timer.add_consumer(&speaker);
        timer.add_consumer(&write);


        std::cout << "Entering process loop" << std::endl;
        while(!wav.is_done())
            timer.tick();
        std::cout << "Exiting" << "\n" << std::endl;
    }
    catch(std::exception& e)
    {
        std::cerr << "\n\n" << "EXCEPTION: " << e.what() << std::endl;
        exit(1);
    }
}
