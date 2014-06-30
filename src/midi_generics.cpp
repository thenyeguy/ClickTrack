#include <cmath>
#include <iostream>
#include "midi_generics.h"

using namespace ClickTrack;


float ClickTrack::midiNoteToFreq(unsigned note)
{
    return 440*pow(2, ((float)note-69)/12);
}




MidiChannel::MidiChannel(MidiGenerator& in_parent, unsigned long start_t)
    : parent(in_parent), last_events(), next_time(start_t)
{}


std::vector<MidiMessage> MidiChannel::get_events(unsigned long t)
{
    // If this block already fell out of the buffer, just return silence
    if(next_time > t+1)
    {
        std::cerr << "MidiChannel has requested a time older than is in "
            << "its buffer." << std::endl;
        last_events.clear();
        return last_events;
    }

    // Otherwise generate enough audio
    while(next_time <= t)
        parent.tick(next_time);

    return last_events; 
}


void MidiChannel::push_events(std::vector<MidiMessage>& events)
{
    last_events = events;
    next_time++;
}




MidiGenerator::MidiGenerator()
    : output_channel(*this), output_frame()
{}


MidiChannel* MidiGenerator::get_output_midi_channel()
{
    return &output_channel;
}


void MidiGenerator::tick(unsigned long t)
{
    output_frame.clear();
    generate_events(output_frame, t);

    //Write the outputs into the channel
    output_channel.push_events(output_frame);
}




MidiConsumer::MidiConsumer()
    : input_channel(nullptr), input_frame()
{}


void MidiConsumer::set_input_midi_channel(MidiChannel* channel)
{
    input_channel = channel;
}


void MidiConsumer::remove_channel()
{
    input_channel = nullptr;
}


void MidiConsumer::tick(unsigned long t)
{
    // If there is no channel currently, read in no events
    input_frame.clear();
    if(input_channel != nullptr)
    {
        input_frame = input_channel->get_events(t);
    }

    // Process
    process_events(input_frame, t);
}


MidiFilter::MidiFilter()
    : MidiGenerator(), MidiConsumer()
{}


void MidiFilter::tick(unsigned long t)
{
    // If there is no channel currently, read in no events
    input_frame.clear();
    if(input_channel != nullptr)
    {
        input_frame = input_channel->get_events(t);
    }

    // Process
    output_frame.clear();
    filter_events(input_frame, output_frame, t);

    //Write the outputs into the channel
    output_channel.push_events(output_frame);
}
