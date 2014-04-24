#include "ring_modulator.h"

using namespace ClickTrack;


RingModulator::RingModulator(float freq, float in_wetness, unsigned num_channels)
    : AudioFilter(num_channels, num_channels),
      modulator(Oscillator::Sine, freq),
      wetness(in_wetness)
{}


void RingModulator::filter(std::vector<SAMPLE>& input,
        std::vector<SAMPLE>& output, unsigned long t)
{
    SAMPLE mod = modulator.get_output_channel()->get_sample(t);
    for(unsigned i=0; i < input.size(); i++)
    {
        SAMPLE in = input[i];
        output[i] = wetness*in*mod + (1-wetness)*in;
    }
}
