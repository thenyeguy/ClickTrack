#include <cmath>
#include <iostream>
#include <random>
#include "oscillator.h"
#include "portaudio_wrapper.h"

using namespace ClickTrack;

Oscillator::Oscillator(Mode in_mode, float in_freq)
    : AudioGenerator(1), 
      last_output(0.0), 
      lfo(nullptr),
      lfo_intensity(0.0),
      modulator(nullptr),
      mod_intensity(0.0),
      master_phase(0.0),
      phase_inc(in_freq * 2*M_PI/SAMPLE_RATE), 
      transpose(1.0),
      mode(in_mode),
      freq(in_freq)
{}


void Oscillator::set_mode(Mode in_mode)
{
    mode = in_mode;
}


void Oscillator::set_freq(float in_freq)
{
    freq = in_freq;
    phase_inc = freq * 2*M_PI/SAMPLE_RATE;
}


void Oscillator::set_transposition(float steps)
{
    transpose = pow(2, steps/12);
}


void Oscillator::set_lfo_input(AudioChannel* input)
{
    lfo = input;
}


void Oscillator::set_lfo_intensity(float steps)
{
    lfo_intensity = steps/12;
}


void Oscillator::set_modulator_input(AudioChannel* input)
{
    modulator = input;
}


void Oscillator::set_modulator_intensity(float intensity)
{
    mod_intensity = intensity;
}


void Oscillator::generate_outputs(std::vector<SAMPLE>& outputs, unsigned long t)
{
    // Compute the LFO contribution
    float lfo_transpose = 1.0;
    if(lfo != nullptr)
        lfo_transpose = pow(2, lfo->get_sample(t) * lfo_intensity);

    // Update the phase
    master_phase += phase_inc * transpose * lfo_transpose;
    if(master_phase > 2*M_PI) master_phase -= 2*M_PI;

    // Get the instantaneous phase
    float phase = master_phase;
    if(modulator != nullptr) 
    {
        phase += mod_intensity * modulator->get_sample(t);
        if(phase > 2*M_PI) phase -= 2*M_PI;
    }

    // Generate this output
    SAMPLE out;
    switch(mode)
    {
        case Sine:
        {
            out = sin(phase);
            break;
        }

        case Saw:
        case BlepSaw:
        {
            out = 2.0*phase/(2*M_PI) - 1.0;

            // one discontinuity, at edge of saw
            if(mode == BlepSaw)
                out -= polyBlepOffset(phase/(2*M_PI));

            break;
        }

        case Square:
        case BlepSquare:
        {
            if(phase < M_PI)
                out = 1.0;
            else
                out = -1.0;

            // two discontinuities, at rising and falling edge
            if(mode == BlepSquare)
            {
                out += polyBlepOffset(phase/(2*M_PI));
                out -= polyBlepOffset(fmod(phase/(2*M_PI) + 0.5, 1.0));
            }

            break;
        }

        case Tri:
        case BlepTri:
        {
            // Compute a square wave signal
            if(phase < M_PI)
                out = 1.0;
            else
                out = -1.0;

            // two discontinuities, at rising and falling edge
            if(mode == BlepTri)
            {
                out += polyBlepOffset(phase/(2*M_PI));
                out -= polyBlepOffset(fmod(phase/(2*M_PI) + 0.5, 1.0));
            }

            // Perform leaky integration of a square wave
            out = phase_inc*out + (1-phase_inc)*last_output;
            last_output = out;
            break;
        }

        case WhiteNoise:
        {
            int si = rand();
            float sf = ((float) si) / RAND_MAX;
            out = 2*sf - 1;
            break;
        }

        case PulseTrain:
        {
            // If we wrapped around...
            if(phase < phase_inc)
                out = 1.0;
            else
                out = 0.0;
            break;
        }
    }
    outputs[0] = out;
}


float Oscillator::polyBlepOffset(float t)
{
    float dt = phase_inc / (2*M_PI);
    if (t < dt)
    {
        t /= dt;
        return t+t - t*t - 1.0;
    }
    else if (t > 1.0 - dt)
    {
        t = (t - 1.0) / dt;
        return t*t + t+t + 1.0;
    }
    else
    {
        return 0.0;
    }
}
