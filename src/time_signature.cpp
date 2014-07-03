#include "audio_generics.h"
#include "time_signature.h"

using namespace ClickTrack;


unsigned TimeSignature::get_tempo()
{
    return tempo;
}


void TimeSignature::set_tempo(unsigned in_tempo)
{
    // Update the tempo
    tempo = in_tempo;
    samples_per_beat = SAMPLE_RATE*60 / tempo;

    // Reset our meter
    current_tick = 0;
}


void TimeSignature::set_meter(meter_t& in_meter)
{
    meter = in_meter;
}


const TimeSignature::meter_t& TimeSignature::get_current_meter()
{
    return meter;
}


bool TimeSignature::is_on_beat()
{
    return current_tick == 0;
}


unsigned TimeSignature::get_current_beat()
{
    return current_beat;
}


TimeSignature::BeatType TimeSignature::get_current_beat_type()
{
    return meter[current_beat];
}


TimeSignature::TimeSignature()
    : tempo(120),
      samples_per_beat(SAMPLE_RATE*60 / tempo),
      current_tick(0),
      meter(),
      current_beat(0)
{
    // Fill the default beat pattern
    meter.push_back(TimeSignature::DOWNBEAT);
    meter.push_back(TimeSignature::UNACCENTED);
    meter.push_back(TimeSignature::UNACCENTED);
    meter.push_back(TimeSignature::UNACCENTED);
}


void TimeSignature::tick()
{
    // March time forward
    current_tick = (current_tick+1) % samples_per_beat;

    // Update our current beat if nessecary
    if(current_tick == 0)
        current_beat = (current_beat+1) % meter.size();
}
