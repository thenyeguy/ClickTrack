#include "timing_manager.h"

using namespace ClickTrack;


TimingManager::TimingManager()
    : time(0),
      consumers(), 
      instruments(),
      last_sync()
{
    last_sync.synced = false;
}


void TimingManager::add_instrument(GenericInstrument* instrument)
{
    instruments.push_back(instrument);
}


void TimingManager::add_consumer(AudioConsumer* consumer)
{
    consumers.push_back(consumer);
}


void TimingManager::tick()
{
    for(auto consumer : consumers)
        consumer->tick(time);
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
