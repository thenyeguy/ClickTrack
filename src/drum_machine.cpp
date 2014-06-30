#include <cstdlib>
#include <string>
#include "wav_reader.h"
#include "drum_machine.h"

using namespace ClickTrack;

DrumMachine::DrumMachine(const std::string& path)
    : volume(-3)
{
    // Create the adder
    adder = new DrumAdder();

    // Connect signal chain
    volume.set_input_channel(adder->get_output_channel());

    // Set the voice
    set_voice(path);
}

void DrumMachine::set_voice(const std::string& path)
{
    adder->set_voice(path);
}


DrumMachine::~DrumMachine()
{
    delete adder;
}


AudioChannel* DrumMachine::get_output_channel()
{
    return volume.get_output_channel();
}


void DrumMachine::on_note_down(unsigned note, float velocity)
{
    adder->on_note_down(note, velocity);
}


// Ignore every message other than note down
void DrumMachine::on_note_up(unsigned note, float velocity) {}
void DrumMachine::on_sustain_down() {}
void DrumMachine::on_sustain_up() {}
void DrumMachine::on_pitch_wheel(float value) {}
void DrumMachine::on_modulation_wheel(float value) {}
void DrumMachine::on_midi_message(MidiMessage message) {}




DrumAdder::DrumAdder()
    : AudioGenerator(1), voices()
{}


DrumAdder::~DrumAdder()
{
    for(auto noteAndVoice: voices)
        delete noteAndVoice.second;
}


void DrumAdder::set_voice(const std::string& path)
{
    // First wipe the old voice
    voices.clear();

    // Open the mapping file
    std::string mapping = path + "keymap.txt";
    std::fstream keymap;
    keymap.open(mapping);

    if(!keymap.good())
        throw InvalidKeymap(mapping);

    // Read each line
    std::string line;
    while(std::getline(keymap, line))
    {
        // Skip blank lines or comment lines
        if(line.size() == 0 || line.front() == '#')
            continue;

        // Parse out the MIDI key and file
        unsigned note = std::strtoul(line.substr(0, line.find(' ')).c_str(), 
                NULL, 0);
        std::string filename = path + line.substr(line.find(' ')+1);

        // Create a drum voice
        voices.insert(std::pair<unsigned,DrumVoice*>
                (note, new DrumVoice(filename)));
    }

    keymap.close();
}


void DrumAdder::generate_outputs(std::vector<SAMPLE>& outputs, unsigned long t)
{
    // Compute the output
    outputs[0] = 0.0; 
    for(auto noteAndVoice: voices)
    {
        // For each voice, get its next sample
        auto voice = noteAndVoice.second;
        if(voice->is_playing())
            outputs[0] += voice->get_next_sample();
    }
}

void DrumAdder::on_note_down(unsigned note, float velocity)
{
    // Ignore if this note doesn't exist
    if(voices.find(note) == voices.end())
        return;

    voices[note]->on_note_down();
}




DrumVoice::DrumVoice(const std::string& filename)
    : samples(), next_i(0), playing(false)
{
    // Read the wavfile into the buffer
    WavReader wav(filename.c_str());

    for(unsigned t=0; t < wav.get_total_samples(); t++)
        samples.push_back(wav.get_output_channel()->get_sample(t));
}


SAMPLE DrumVoice::get_next_sample()
{
    // Generate sample
    if(playing)
    {
        SAMPLE out = samples[next_i];
        next_i++;

        if(next_i >= samples.size())
            playing = false;

        return out;
    }
    else
    {
        return 0.0;
    }
}


void DrumVoice::on_note_down()
{
    // Set state
    next_i = 0;
    playing = true;
}


bool DrumVoice::is_playing()
{
    return playing;
}
