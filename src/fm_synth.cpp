#include <iterator>
#include <cmath>
#include "fm_synth.h"

using namespace ClickTrack;


FMSynth::FMSynth(int num_voices)
    : PolyphonicInstrument(num_voices), 
      filter(SecondOrderFilter::LOWPASS, 20000),
      lfo(Oscillator::Sine, 5),
      volume(-10)
{
    // Initialize our voices
    std::vector<PolyphonicVoice*> temp;
    for(unsigned i = 0; i < num_voices; i++)
    {
        FMSynthVoice* voice = new FMSynthVoice(this);
        voices.push_back(voice);
        temp.push_back(voice);
    }
    add_voices(temp);

    // Configure our signal chain
    filter.set_input_channel(PolyphonicInstrument::get_output_channel());
    volume.set_input_channel(filter.get_output_channel());

    // Connect the LFO
    for(auto voice : voices)
        voice->carrier.set_lfo_input(lfo.get_output_channel());

    volume.set_lfo_input(lfo.get_output_channel());
}


AudioChannel* FMSynth::get_output_channel()
{
    return volume.get_output_channel();
}


void FMSynth::set_gain(float in_gain)
{
    volume.set_gain(in_gain);
}


void FMSynth::set_carrier_mode(Oscillator::Mode mode)
{
    for(auto voice : voices)
        voice->carrier.set_mode(mode);
}


void FMSynth::set_modulator_mode(Oscillator::Mode mode)
{
    for(auto voice : voices)
        voice->modulator.set_mode(mode);
}


void FMSynth::set_carrier_transposition(float steps)
{
    for(auto voice : voices)
        voice->carrier.set_transposition(steps);
}


void FMSynth::set_modulator_transposition(float steps)
{
    for(auto voice : voices)
        voice->modulator.set_transposition(steps);
}


void FMSynth::set_modulator_intensity(float intensity)
{
    for(auto voice : voices)
        voice->carrier.set_modulator_intensity(intensity);
}


void FMSynth::set_attack_time(float attack_time)
{
    for(auto voice : voices)
        voice->adsr.set_attack_time(attack_time);
}


void FMSynth::set_decay_time(float decay_time)
{
    for(auto voice : voices)
        voice->adsr.set_decay_time(decay_time);
}


void FMSynth::set_sustain_level(float sustain_level)
{
    for(auto voice : voices)
        voice->adsr.set_sustain_level(sustain_level);
}


void FMSynth::set_release_time(float release_time)
{
    for(auto voice : voices)
        voice->adsr.set_release_time(release_time);
}


void FMSynth::set_lfo_vibrato(float steps)
{
    for(auto voice : voices)
        voice->carrier.set_lfo_intensity(steps);
}


void FMSynth::set_lfo_tremolo(float db)
{
    volume.set_lfo_intensity(db);
}




FMSynthVoice::FMSynthVoice(FMSynth* in_parent_synth)
    : PolyphonicVoice(in_parent_synth),
      carrier(Oscillator::Sine, 440), 
      modulator(Oscillator::Sine, 440), 
      adsr()
{
    // Connect signal chain
    carrier.set_modulator_input(modulator.get_output_channel());
    adsr.set_input_channel(carrier.get_output_channel());
}


AudioChannel* FMSynthVoice::get_output_channel()
{
    return adsr.get_output_channel();
}


void FMSynthVoice::handle_note_down(float velocity)
{
    // Set velocity gain
    adsr.set_gain(pow(velocity,0.5));

    // Trigger frequency and ADSR change
    carrier.set_freq(freq*pitch_multiplier);
    modulator.set_freq(freq*pitch_multiplier);
    adsr.on_note_down();
}


void FMSynthVoice::handle_note_up()
{
    adsr.on_note_up();
}


void FMSynthVoice::handle_pitch_wheel(float value)
{
    carrier.set_freq(freq*pitch_multiplier);
    modulator.set_freq(freq*pitch_multiplier);
}
