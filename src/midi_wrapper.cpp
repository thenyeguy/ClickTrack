#include <cmath>
#include <iostream>
#include <iomanip>
#include "generic_instrument.h"
#include "midi_wrapper.h"

using namespace ClickTrack;
namespace chr = std::chrono;


MidiListener::MidiListener(TimingManager& in_timer, GenericInstrument& in_inst, 
        int channel)
    : stream(), inst(in_inst), timer(in_timer)
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


void MidiListener::midi_callback(double deltaTime, std::vector<unsigned char>* message,
                      void* in_listener)
{
    MidiListener* listener = (MidiListener*) in_listener;
    if(message->size() == 0)
    {
        std::cerr << "Ignoring empty MIDI message." << std::endl;
        return;
    }

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

    // Cast listener to correct type, then case on message type
    GenericInstrument& inst = listener->inst;
    unsigned char first = message->at(0);
    unsigned char type = first >> 4;
    //unsigned char channel = first & 0x0F;
    switch(type)
    {
        case 0x9: // Note down
        {
            unsigned char note = message->at(1);
            float veloc = double(message->at(2))/100;
            inst.on_note_down(note, veloc, time);
            break;
        }

        case 0x8: // Note up
        {
            unsigned char note = message->at(1);
            float veloc = double(message->at(2))/100;
            inst.on_note_up(note, veloc, time);
            break;
        }

        case 0xB: // Control message 
        {
            switch(message->at(1))
            {
                case 0x01: // modulation wheel
                {
                    float value = (float)message->at(2) / 127;
                    inst.on_modulation_wheel(value, time);
                    break;
                }

                case 0x40: // sustain pedal
                {
                    if(message->at(2) < 63)
                        inst.on_sustain_up(time);
                    else
                        inst.on_sustain_down(time);

                    break;
                }

                default:
                    goto UNHANDLED;
                    break;
            }
                
            break;
        }

        case 0xE: // Pitch wheel
        {
            // Convert to float between -1.0 and 1.0
            unsigned value = (message->at(2) << 7) | message->at(1);
            int centered = value - 0x2000;
            float bend = (float)centered / 0x2000;

            inst.on_pitch_wheel(bend, time);
            break;
        }
        
        UNHANDLED:
        default:
        {
            std::cout << "Unknown messsage: 0x";
            for(int i=0; i < message->size(); i++)
                std::cout << std::hex << std::setfill('0') << std::setw(2) << 
                    (unsigned) message->at(i);
            std::cout << std::endl;

            inst.on_midi_message(message, time);
        }
    }
}
