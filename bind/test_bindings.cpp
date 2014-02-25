#include <exception>
#include "../src/logcat.h"
#include "test_bindings.h"

void Java_edu_cmu_ece_ece551_clicktrack_TestClickTrack_initOpenSlesTest(
        JNIEnv* env, jobject obj)
{
    try
    {
        logi("Initializing ClickTrack signal chain\n");
        ClickTrack::Microphone mic;
        ClickTrack::Speaker speak;
        speak.set_input_channel(mic.get_output_channel());

        logi("Beginning playback\n");
        while(true)
            speak.consume_inputs();

    }
    catch(std::exception& e)
    {
        // Log and die
        loge("Exception caught during testing: %s\n",
                e.what());
        exit(1);
    }
}
