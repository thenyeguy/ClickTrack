#include <iostream>
#include "adsr.h"

using namespace ClickTrack;


ADSRFilter::ADSRFilter(float in_attack_time, float in_decay_time,
                       float in_sustain_level, float in_release_time,
                       float in_gain, unsigned in_num_channels)
    : AudioFilter(in_num_channels, in_num_channels), scheduler(*this),
      state(silent), t(0), trigger_time(0), end_time(0), multiplier(0), 
      delta_mult(0)
{
    attack_time  = in_attack_time  * SAMPLE_RATE;
    decay_time   = in_decay_time   * SAMPLE_RATE;
    release_time = in_release_time * SAMPLE_RATE;
    gain = in_gain;

    sustain_level = in_sustain_level;
}


void ADSRFilter::on_note_down(unsigned long time)
{
    if(time == 0)
        time = next_t;

    // Run the callback with no payload
    scheduler.schedule(time, ADSRFilter::note_down_callback, NULL);
}


void ADSRFilter::on_note_up(unsigned long time)
{
    if(time == 0)
        time = next_t;

    // Run the callback with no payload
    scheduler.schedule(time, ADSRFilter::note_up_callback, NULL);
}


void ADSRFilter::note_down_callback(ADSRFilter& caller, void* payload)
{
    // Ignore payload
    // Set the state
    caller.state = attack;
    caller.trigger_time = caller.t;
    caller.end_time = caller.t + caller.attack_time;
    
    // Recalculate the multiplier delta
    caller.delta_mult = (1.0 - caller.multiplier)/caller.attack_time;
}


void ADSRFilter::note_up_callback(ADSRFilter& caller, void* payload)
{
    // Ignore payload
    // Set the state
    caller.state = release;
    caller.trigger_time = caller.t;
    caller.end_time = caller.t + caller.release_time;

    // Recalculate the multiplier delta
    caller.delta_mult = (0.0 - caller.multiplier)/caller.release_time;
}


void ADSRFilter::set_attack_time(float in)
{
    attack_time = in;
}


void ADSRFilter::set_decay_time(float in)
{
    decay_time = in;
}


void ADSRFilter::set_sustain_level(float in)
{
    sustain_level = in;
}


void ADSRFilter::set_release_time(float in)
{
    release_time = in;
}


void ADSRFilter::set_gain(float in)
{
    gain = in;
}


void ADSRFilter::filter(std::vector< std::vector<SAMPLE> >& input,
        std::vector< std::vector<SAMPLE> >& output)
{
    for(int i = 0; i < FRAME_SIZE; i++)
    {
        // Update the time
        t = next_t+i;

        // Run the scheduler
        scheduler.run(t);

        // Update the multiplier
        multiplier += delta_mult;

        // Update the state if nessecary
        if(t >= end_time)
        {
            switch(state)
            {
                case attack:
                    state = decay;
                    trigger_time = t;
                    end_time = t + decay_time;

                    multiplier = 1.0;
                    delta_mult = (sustain_level - 1.0)/decay_time;
                    break;

                case decay:
                    state = sustain;
                    trigger_time = t;
                    end_time = t;

                    multiplier = sustain_level;
                    delta_mult = 0.0;
                    break;

                case release:
                    state = silent;
                    trigger_time = t;
                    end_time = t;

                    multiplier = 0.0;
                    delta_mult = 0.0;
                    break;
                
                default:
                    // Do nothing, no other states end themselves
                    break;
            }
        }


        // Copy the new set of samples to our output
        for(int j = 0; j < num_input_channels; j++)
        {
            output[j][i] = gain * multiplier * input[j][i];
        }
    }
}
