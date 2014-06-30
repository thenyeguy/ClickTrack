#include <cmath>
#include <iomanip>
#include <iostream>
#include "generic_instrument.h"

using namespace ClickTrack;


GenericInstrument::GenericInstrument()
    : MidiConsumer(), output_channels()
{}


void GenericInstrument::add_output_channel(AudioChannel* channel)
{
    output_channels.push_back(channel);
}


AudioChannel* GenericInstrument::get_output_channel(int channel)
{
    if(channel > output_channels.size())
        throw AudioChannelOutOfRange();
    return output_channels[channel];
}


const unsigned GenericInstrument::get_num_output_channels()
{
    return output_channels.size();
}


void GenericInstrument::process_events(std::vector<MidiMessage>& inputs, 
        unsigned long t)
{
    for(auto input : inputs)
    {
        switch(input.type)
        {
            case NOTE_DOWN:
            {
                unsigned char note = input.message[0];
                float veloc = double(input.message[1])/100.0;
                on_note_down(note, veloc);
                break;
            }

            case NOTE_UP:
            {
                unsigned char note = input.message[0];
                float veloc = double(input.message[1])/100.0;
                on_note_up(note, veloc);
                break;
            }

            case CONTROL_CHANGE:
            {
                switch(input.message[0])
                {
                    case 0x01: // modulation wheel
                    {
                        float value = (float)input.message[1] / 127.0;
                        on_modulation_wheel(value);
                        break;
                    }

                    case 0x40: // sustain pedal
                    {
                        if(input.message[1] < 63)
                            on_sustain_up();
                        else
                            on_sustain_down();
                        break;
                    }

                    default:
                        goto UNHANDLED;
                }
                break;
            }

            case PITCH_BEND:
            {
                // Convert to float between -1.0 and 1.0
                unsigned value = (input.message[1] << 7) | input.message[0];
                int centered = value - 0x2000;
                float bend = (float)centered / 0x2000;

                on_pitch_wheel(bend);
                break;
            }

            UNHANDLED:
            default:
            {
                // Print out raw message
                std::cout << "Unknown messsage: 0x";
                std::cout << std::hex << std::setfill('0') << std::setw(2) << 
                    (unsigned) input.type << (unsigned) input.channel;
                for(int i=0; i < input.message.size(); i++)
                    std::cout << std::hex << std::setfill('0') << std::setw(2) << 
                        (unsigned) input.message[i];
                std::cout << std::endl;

                on_midi_message(input);
            }
        }
    }
}
