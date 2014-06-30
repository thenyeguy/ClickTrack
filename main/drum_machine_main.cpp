#include "../src/clip_detector.h"
#include "../src/drum_machine.h"
#include "../src/speaker.h"
#include "../src/midi_wrapper.h"
#include "../src/timing_manager.h"

using namespace ClickTrack;

int main()
{
    using namespace std;

    cout << "Initializing MIDI instrument" << endl;
    TimingManager timing;

    DrumMachine drum("samples/roland808/");
    timing.add_instrument(&drum);

    MidiListener midi(timing, drum, 1);

    cout << "Initializing signal chain" << endl;
    ClipDetector clip(1.0);
    clip.set_input_channel(drum.get_output_channel());

    Speaker out(timing);
    out.set_input_channel(clip.get_output_channel());
    timing.add_consumer(&out);

    cout << "Entering playback loop..." << endl << endl;
    while(true)
        timing.tick();

    return 0;
}
