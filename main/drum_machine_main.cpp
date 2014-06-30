#include "../src/clip_detector.h"
#include "../src/drum_machine.h"
#include "../src/speaker.h"
#include "../src/midi_listener.h"
#include "../src/timing_manager.h"

using namespace ClickTrack;

int main()
{
    using namespace std;

    std::cout << "Initializing MIDI instrument" << std::endl;
    TimingManager timing;

    MidiListener midi(timing, 1);
    DrumMachine drum("samples/roland808/");
    drum.set_input_midi_channel(midi.get_output_midi_channel());
    timing.add_midi_consumer(&drum);

    cout << "Initializing signal chain" << endl;
    ClipDetector clip(1.0);
    clip.set_input_channel(drum.get_output_channel());

    Speaker out(timing);
    out.set_input_channel(clip.get_output_channel());
    timing.add_audio_consumer(&out);

    cout << "Entering playback loop..." << endl << endl;
    while(true)
        timing.tick();

    return 0;
}
