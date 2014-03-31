#include "microphone.h"

using namespace ClickTrack;


Microphone::Microphone(unsigned num_channels)
    : AudioGenerator(num_channels), buffer(),
      openSles(OpenSlesWrapper::get_instance())
{
    for(unsigned i = 0; i < num_channels; i++)
        buffer.push_back(std::vector<SAMPLE>(BUFFER_SIZE));
}


void Microphone::generate_outputs(std::vector<SAMPLE>& outputs, unsigned long t)
{
    // If we have run out of samples, refill our buffer
    if(t % BUFFER_SIZE == 0)
        openSles.read_inputs(buffer);

    // Copy one frame out
    for(unsigned i = 0; i < outputs.size(); i++)
        outputs[i] = buffer[i][t % BUFFER_SIZE];
}
