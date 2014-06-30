#include <iostream>
#include "audio_generics.h"


using namespace ClickTrack;


AudioChannel::AudioChannel(AudioGenerator& in_parent, unsigned long start_t)
    : parent(in_parent), last_sample(0.0), next_time(start_t)
{}


SAMPLE AudioChannel::get_sample(unsigned long t)
{
    // If this block already fell out of the buffer, just return silence
    if(next_time > t+1)
    {
        std::cerr << "AudioChannel has requested a time older than is in "
            << "its buffer." << std::endl;
        return 0.0;
    }

    // Otherwise generate enough audio
    while(next_time <= t)
        parent.tick(next_time);

    return last_sample; 
}


void AudioChannel::push_sample(SAMPLE s)
{
    last_sample = s;
    next_time++;
}




AudioGenerator::AudioGenerator(unsigned in_num_output_channels)
    : output_channels(), output_frame()
{
    for(unsigned i = 0; i < in_num_output_channels; i++)
    {
        output_channels.push_back(AudioChannel(*this));
        output_frame.push_back(0.0);
    }
}


unsigned AudioGenerator::get_num_output_channels()
{
    return output_channels.size();
}


AudioChannel* AudioGenerator::get_output_channel(unsigned i)
{
    if(i >= output_channels.size())
        throw AudioChannelOutOfRange();
    return &output_channels[i];
}


void AudioGenerator::tick(unsigned long t)
{
    generate_outputs(output_frame, t);

    //Write the outputs into the channel
    for(unsigned i = 0; i < output_channels.size(); i++)
        output_channels[i].push_sample(output_frame[i]);
}




AudioConsumer::AudioConsumer(unsigned in_num_input_channels)
    : input_channels(in_num_input_channels, NULL), input_frame()
{
    for(unsigned i = 0; i < in_num_input_channels; i++)
        input_frame.push_back(0.0);
}


void AudioConsumer::set_input_channel(AudioChannel* channel, unsigned channel_i)
{
    input_channels[channel_i] = channel;
}


void AudioConsumer::remove_channel(unsigned channel_i)
{
    input_channels[channel_i] = NULL;
}


unsigned AudioConsumer::get_channel_index(AudioChannel* channel)
{
    for(unsigned i = 0; i < input_channels.size(); i++)
    {
        if(input_channels[i] == channel)
            return i;
    }

    throw AudioChannelNotFound();
}


unsigned AudioConsumer::get_num_input_channels()
{
    return input_channels.size();
}


void AudioConsumer::tick(unsigned long t)
{
    // Read in each channel
    for(unsigned i = 0; i < input_channels.size(); i++)
    {
        // If there is no channel currently, read in silence
        if(input_channels[i] == NULL)
        {
            //std::cerr << "The requested channel is not connected" << std::endl;
            input_frame[i] = 0.0;
        }
        else
        {
            input_frame[i] = input_channels[i]->get_sample(t);
        }
    }

    // Process
    process_inputs(input_frame, t);
}




AudioFilter::AudioFilter(unsigned in_num_input_channels,
        unsigned in_num_output_channels)
    : AudioGenerator(in_num_output_channels),
      AudioConsumer(in_num_input_channels)
{}


void AudioFilter::tick(unsigned long t)
{
    // Read in each channel
    for(unsigned i = 0; i < input_channels.size(); i++)
    {
        // If there is no channel currently, read in silence
        if(input_channels[i] == NULL)
        {
            //std::cerr << "The requested channel is not connected" << std::endl;
            input_frame[i] = 0.0;
        }
        else
        {
            input_frame[i] = input_channels[i]->get_sample(t);
        }
    }

    // Process
    filter(input_frame, output_frame, t);

    //Write the outputs into the channel
    for(unsigned i = 0; i < output_channels.size(); i++)
        output_channels[i].push_sample(output_frame[i]);
}
