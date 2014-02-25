#include <ctime>
#include "logcat.h" 
#include "opensles_wrapper.h"

using namespace ClickTrack;


OpenSlesWrapper& OpenSlesWrapper::get_instance()
{
    static OpenSlesWrapper instance;
    return instance;
}


void OpenSlesWrapper::write_outputs(std::vector< std::vector<SAMPLE> >& outputs)
{
    // Fill our output buffer and interleave
    for(unsigned i = 0; i < BUFFER_SIZE; i++)
    {
        for(unsigned j = 0; j < num_channels; j++)
        {
            // Automatically handle mono input
            SAMPLE sample;
            if(outputs.size() == 1)
                sample = outputs[0][i];
            else
                sample = outputs[j][i];

            // Clip instead of overflowing
            if(sample > 0.999f)  sample = 0.999f;
            if(sample < -0.999f) sample = -0.999f;

            // Save in buffer
            output_buffer[num_channels*i + j] = sample;
        }
    }

    // Send result out to stream
    android_AudioOut(stream, output_buffer, num_channels*BUFFER_SIZE);
}


void OpenSlesWrapper::read_inputs(std::vector< std::vector<SAMPLE> >& inputs)
{
    // Fill the input buffer
    android_AudioIn(stream, input_buffer, num_channels*BUFFER_SIZE);

    // Deinterleave and write the buffer out
    for(unsigned i = 0; i < BUFFER_SIZE; i++)
    {
        for(unsigned j = 0; j < inputs.size(); j++)
        {
            // Automatically handle stereo output
            SAMPLE in_sample;
            if(num_channels == 1)
                in_sample = input_buffer[i];
            else
                in_sample = input_buffer[num_channels*i + j];

            // Save to output vector
            inputs[j][i] = in_sample;
        }
    }
}


OpenSlesWrapper::OpenSlesWrapper()
    : num_channels(1)
{
    logi("Initializing OpenSL ES wrapper");

    // Allocate our input buffers
    output_buffer = new SAMPLE[BUFFER_SIZE*num_channels];
    input_buffer = new SAMPLE[BUFFER_SIZE*num_channels];

    // Open our stream object
    stream = android_OpenAudioDevice(SAMPLE_RATE, num_channels, num_channels, 
            BUFFER_SIZE);

    logi("Successfully initialized OpenSL ES wrapper");
}


OpenSlesWrapper::~OpenSlesWrapper()
{
    logi("Destroying OpenSL ES wrapper");

    // CLse our stream object
    android_CloseAudioDevice(stream);

    // Release our buffers
    delete output_buffer;
    delete input_buffer;

    logi("Successfully destroyed OpenSL ES wrapper");
}
