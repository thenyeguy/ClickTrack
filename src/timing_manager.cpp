#include "timing_manager.h"

using namespace ClickTrack;


TimingManager::TimingManager()
    : rhythm_manager(),
      time(0),
      midi_consumers(), 
      audio_consumers(),
      last_sync()
{
    // Set unsynced
    last_sync.synced = false;
}


void TimingManager::add_midi_consumer(MidiConsumer* consumer)
{
    midi_consumers.push_back(consumer);
}


void TimingManager::add_audio_consumer(AudioConsumer* consumer)
{
    audio_consumers.push_back(consumer);
}


void TimingManager::tick()
{
    // Tick the consumers
    for(auto consumer : midi_consumers)
        consumer->tick(time);
    for(auto consumer : audio_consumers)
        consumer->tick(time);

    // Tick time forward
    rhythm_manager.tick();
    time++;
}


unsigned long TimingManager::get_current_time()
{
    return time;
}


void TimingManager::synchronize(unsigned long t)
{
    last_sync = {true, t, std::chrono::high_resolution_clock::now()};
}


TimingManager::SynchronizationStatus TimingManager::get_last_synchronization()
{
    return last_sync;
}
