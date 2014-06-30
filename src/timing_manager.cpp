#include "timing_manager.h"

using namespace ClickTrack;


TimingManager::TimingManager()
    : consumers(), instruments(), time(0)
{}


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
