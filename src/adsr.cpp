#include <cmath>
#include <iostream>
#include "adsr.h"

using namespace ClickTrack;


ADSRFilter::ADSRFilter(float in_attack_time, float in_decay_time,
                       float in_sustain_level, float in_release_time,
                       float in_gain, unsigned in_num_channels)
    : AudioFilter(in_num_channels, in_num_channels),
      state(silent), state_time(0), state_duration(0), multiplier(0), 
      delta_mult(0)
{
    attack_time  = in_attack_time  * SAMPLE_RATE;
    decay_time   = in_decay_time   * SAMPLE_RATE;
    release_time = in_release_time * SAMPLE_RATE;
    gain = pow(10,in_gain/10);

    sustain_level = in_sustain_level;
}


void ADSRFilter::on_note_down()
{
    state = attack;
    state_time = 0;
    state_duration = attack_time;
}


void ADSRFilter::on_note_up()
{
    state = release;
    state_time = 0;
    state_duration = release_time;
}


void ADSRFilter::set_attack_time(float in)
{
    attack_time = SAMPLE_RATE*in;
}


void ADSRFilter::set_decay_time(float in)
{
    decay_time = SAMPLE_RATE*in;
}


void ADSRFilter::set_sustain_level(float in)
{
    sustain_level = in;
}


void ADSRFilter::set_release_time(float in)
{
    release_time = SAMPLE_RATE*in;
}


void ADSRFilter::set_gain(float in_gain)
{
    gain = pow(10,in_gain/10);
}


void ADSRFilter::filter(std::vector<SAMPLE>& input, std::vector<SAMPLE>& output,
        unsigned long t)
{
    // Update the multiplier
    multiplier += delta_mult;

    // Update the state if nessecary
    state_time++;
    if(state_time > state_duration)
    {
        switch(state)
        {
            case attack:
                state = decay;
                state_time = 0;
                state_duration = decay_time;

                multiplier = 1.0;
                delta_mult = (sustain_level - 1.0)/decay_time;
                break;

            case decay:
                state = sustain;
                state_time = 0;
                state_duration = 0;

                multiplier = sustain_level;
                delta_mult = 0.0;
                break;

            case release:
                state = silent;
                state_time = 0;
                state_duration = 0;

                multiplier = 0.0;
                delta_mult = 0.0;
                break;

            default:
                // Do nothing, no other states end themselves
                break;
        }
    }


    // Copy the new set of samples to our output
    for(int i = 0; i < input.size(); i++)
    {
        output[i] = gain * multiplier * input[i];
    }
}
