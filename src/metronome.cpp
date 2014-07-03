#include "wav_reader.h"
#include "metronome.h"

using namespace ClickTrack;


Metronome::Metronome(TimingManager& timing_manager,
        const std::string& downbeat_sound,
        const std::string& accented_sound,
        const std::string& unaccented_sound)
    : AudioGenerator(1),
      rhythm_manager(timing_manager.rhythm_manager),
      downbeat(),
      accented(),
      unaccented(),
      current_sample(0),
      current_beat(nullptr)
{
    // Read in the click sounds
    WavReader downbeat_wav(downbeat_sound.c_str());
    downbeat.reserve(downbeat_wav.get_total_samples());
    for(unsigned t = 0; t < downbeat_wav.get_total_samples(); t++)
        downbeat.push_back(downbeat_wav.get_output_channel()->get_sample(t));

    WavReader accented_wav(accented_sound.c_str());
    accented.reserve(accented_wav.get_total_samples());
    for(unsigned t = 0; t < accented_wav.get_total_samples(); t++)
        accented.push_back(accented_wav.get_output_channel()->get_sample(t));

    WavReader unaccented_wav(unaccented_sound.c_str());
    unaccented.reserve(unaccented_wav.get_total_samples());
    for(unsigned t = 0; t < unaccented_wav.get_total_samples(); t++)
        unaccented.push_back(unaccented_wav.get_output_channel()->get_sample(t));
}


void Metronome::generate_outputs(std::vector<SAMPLE>& output, unsigned long t)
{
    // First trigger click if on beat
    if(rhythm_manager.is_on_beat())
    {
        current_sample = 0;
        switch(rhythm_manager.get_current_beat_type())
        {
            case RhythmManager::DOWNBEAT:
                current_beat = &downbeat;
                break;
            case RhythmManager::ACCENTED:
                current_beat = &accented;
                break;
            case RhythmManager::UNACCENTED:
                current_beat = &unaccented;
                break;
        }
    }

    // If we are still playing a click, return that
    // else return silence
    if(current_sample < current_beat->size())
    {
        output[0] = current_beat->at(current_sample);
            current_sample++;
    }
    else
    {
        output[0] = 0.0;
    }
}
