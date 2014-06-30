#include <cstdlib>
#include <iostream>
#include "../src/microphone.h"
#include "../src/moorer_reverb.h"
#include "../src/speaker.h"
#include "../src/timing_manager.h"

using namespace ClickTrack;


int main()
{
    try
    {
        std::cout << "Establishing signal chain" << std::endl;
        Microphone mic;

        MoorerReverb rev(MoorerReverb::HALL, 1.0, 0.3, 0.5, 1);
        rev.set_input_channel(mic.get_output_channel());

        Speaker out;
        out.set_input_channel(rev.get_output_channel());

        TimingManager timer;
        timer.add_consumer(&out);


        std::cout << "Playing" << std::endl;
        while(true)
            timer.tick();

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
