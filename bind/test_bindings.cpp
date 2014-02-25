#include <exception>
#include "../src/logcat.h"
#include "../src/oscillator.h"
#include "test_bindings.h"

void Java_edu_cmu_ece_ece551_clicktrack_TestClickTrack_initOpenSlesTest(
        JNIEnv* env, jobject obj)
{
    try
    {
        logi("Initializing ClickTrack signal chain");
        ClickTrack::Microphone gen;
        //ClickTrack::SawWave gen(440);
        ClickTrack::Speaker speak;
        speak.set_input_channel(gen.get_output_channel());

        logi("Beginning playback");
        while(true)
        {
            speak.consume_inputs();
        }

    }
    catch(std::exception& e)
    {
        // Log and die
        loge("Exception caught during testing: %s",
                e.what());
        exit(1);
    }
}
