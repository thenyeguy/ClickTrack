#include "audio_generics.h"
#include "rhythm_manager.h"

using namespace ClickTrack;


unsigned RhythmManager::get_tempo()
{
    return tempo;
}


void RhythmManager::set_tempo(unsigned in_tempo)
{
    // Update the tempo
    tempo = in_tempo;
    samples_per_beat = SAMPLE_RATE*60 / tempo;

    // Reset our meter
    current_tick = 0;
}


void RhythmManager::set_meter(meter_t& in_meter)
{
    meter = in_meter;
}


const RhythmManager::meter_t& RhythmManager::get_current_meter()
{
    return meter;
}


bool RhythmManager::is_on_beat()
{
    return (current_tick % samples_per_beat) == 0;
}


unsigned RhythmManager::get_current_beat()
{
    return current_beat;
}


RhythmManager::BeatType RhythmManager::get_current_beat_type()
{
    return meter[current_beat];
}


bool RhythmManager::is_beat_subdivision(unsigned numerator, 
        unsigned denominator)
{
    return (current_tick % 
            (samples_per_beat*denominator/numerator)) == 0;
}


bool RhythmManager::is_measure_subdivision(unsigned numerator, 
        unsigned denominator)
{
    return (current_tick % 
            (samples_per_beat*meter.size()*denominator/numerator)) == 0;
}


RhythmManager::RhythmManager()
    : tempo(120),
      samples_per_beat(SAMPLE_RATE*60 / tempo),
      meter(),
      current_beat(0),
      current_tick(0)
{
    // Fill the default beat pattern
    meter.push_back(RhythmManager::DOWNBEAT);
    meter.push_back(RhythmManager::UNACCENTED);
    meter.push_back(RhythmManager::UNACCENTED);
    meter.push_back(RhythmManager::UNACCENTED);
}


void RhythmManager::tick()
{
    // March time forward
    current_tick = (current_tick+1) % (samples_per_beat*meter.size());

    // Update our current beat if nessecary
    if((current_tick % samples_per_beat) == 0)
        current_beat = (current_beat+1) % meter.size();
}
