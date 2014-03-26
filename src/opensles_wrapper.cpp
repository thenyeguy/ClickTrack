#include <ctime>
#include "logcat.h" 
#include "opensles_wrapper.h"

using namespace ClickTrack;


unsigned ClickTrack::BUFFER_SIZE = 1024;


OpenSlesWrapper& OpenSlesWrapper::get_instance()
{
    static OpenSlesWrapper instance;
    return instance;
}


void OpenSlesWrapper::start()
{
    // Only start if stream is not already running
    if(stream != nullptr)
        return;

    logi("Starting OpenSL ES playback with buffer size: %u", BUFFER_SIZE);
    stream = android_OpenAudioDevice(SAMPLE_RATE, num_channels, num_channels, 
            BUFFER_SIZE);
}


void OpenSlesWrapper::stop()
{
    // Only stop if stream is running
    if(stream == nullptr)
        return;

    logi("Stopping OpenSL ES playback");
    android_CloseAudioDevice(stream);
    stream = nullptr;
}


void OpenSlesWrapper::write_outputs(std::vector< std::vector<SAMPLE> >& outputs)
{
    // Check for valid stream
    if(stream == nullptr)
    {
        loge("Trying to write to closed OpenSLES stream.");
        exit(1);
    }

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
    // Check for valid stream
    if(stream == nullptr)
    {
        loge("Trying to read from closed OpenSLES stream.");
        exit(1);
    }

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
    
    // Start with no stream
    stream = nullptr;
}


OpenSlesWrapper::~OpenSlesWrapper()
{
    logi("Destroying OpenSL ES wrapper");

    // Stop the stream
    stop();

    // Release our buffers
    delete output_buffer;
    delete input_buffer;
}
