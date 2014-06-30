#include <cmath>
#include "generic_instrument.h"

using namespace ClickTrack;


float ClickTrack::midiNoteToFreq(unsigned note)
{
    return 440*pow(2, ((float)note-69)/12);
}


GenericInstrument::GenericInstrument()
    : output_channels()
{}


void GenericInstrument::add_output_channel(AudioChannel* channel)
{
    output_channels.push_back(channel);
}


AudioChannel* GenericInstrument::get_output_channel(int channel)
{
    if(channel > output_channels.size())
        throw AudioChannelOutOfRange();
    return output_channels[channel];
}

const unsigned GenericInstrument::get_num_output_channels()
{
    return output_channels.size();
}
