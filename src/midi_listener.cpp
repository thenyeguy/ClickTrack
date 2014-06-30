#include <cmath>
#include <iostream>
#include <iomanip>
#include "generic_instrument.h"
#include "midi_listener.h"

using namespace ClickTrack;
namespace chr = std::chrono;


MidiListener::MidiListener(TimingManager& in_timer, int channel)
    : stream(), timer(in_timer), event_i(0), events()
{
    // If no channel specified, ask the user for a channel
    if(channel == -1)
    {
        // First list all the channels
        unsigned nPorts = stream.getPortCount();
        std::cout << std::endl << "There are " << nPorts <<
                " MIDI input sources available." << std::endl;
        for(int i=0; i<nPorts; i++)
            std::cout << "  Input Port #" << i << ": " <<
                    stream.getPortName(i) << std::endl;

        // Ask for a channel
        while(true)
        {
            std::cout << "Choose a MIDI channel: ";
            std::cin >> channel;
            if(0 <= channel && channel < nPorts)
                break;

            std::cerr << "    Not a valid channel number." << std::endl;
        }
    }

    // Once we have a channel, initialize it
    stream.setCallback(&MidiListener::midi_callback, this);
    stream.openPort(channel);
}


void MidiListener::generate_events(std::vector<MidiMessage>& outputs,
        unsigned long t)
{
    // Check for untriggered events at this time
    while(!events.empty() && events.top().t <= t)
    {
        // Get the event
        struct event_t event = events.top();
        events.pop();
        
        // Push it to our outgoing events
        outputs.push_back(event.m);
    }
}


void MidiListener::midi_callback(double deltaTime,
        std::vector<unsigned char>* in_message, void* in_listener)
{
    MidiListener* listener = (MidiListener*) in_listener;
    if(in_message->size() == 0)
    {
        std::cerr << "Ignoring empty MIDI message." << std::endl;
        return;
    }

    // Break out the actual message
    unsigned char first = in_message->at(0);
    unsigned char type = first >> 4;
    unsigned char channel = first & 0x0F;
    in_message->erase(in_message->begin());

    // Package into a MidiMessage
    MidiMessage message;
    message.type = (MidiMessageType) type;
    message.channel = channel;
    message.message = *in_message;


    // Get the offset, delay by one frame, if we have received callback info
    unsigned long time = listener->timer.get_current_time();
    TimingManager::SynchronizationStatus sync = 
        listener->timer.get_last_synchronization();
    if(sync.synced) 
    {
        auto diff = chr::high_resolution_clock::now() - 
            sync.timestamp;
        double nanos = chr::duration_cast<chr::nanoseconds>(diff).count();
        unsigned long delay = nanos / 1e9 * SAMPLE_RATE;
        time = sync.sample_time + BUFFER_SIZE + delay;
    }


    // Package the message into an event and push it to the queue
    listener->events.push( {listener->event_i, time, message} );
    listener->event_i++;
}
