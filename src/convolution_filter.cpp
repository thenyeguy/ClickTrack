#include <iostream>
#include <complex>
#include "convolution_filter.h"

using namespace ClickTrack;


ConvolutionFilter::ConvolutionFilter(unsigned impulse_length,
                                     SAMPLE* in_impulse_response)
    : AudioFilter(1),

      transform_size(CONVOLUTION_BUFFER_SIZE*2),
      transformer(transform_size),

      num_impulse_blocks((impulse_length - 1) / 
                         (transform_size-CONVOLUTION_BUFFER_SIZE) + 1),
        // shift by one so perfect powers of two don't get overallocated
      impulse_response(num_impulse_blocks, NULL),

      frequency_buffer(num_impulse_blocks),
      reverb_buffer(transform_size),

      output_queue(CONVOLUTION_BUFFER_SIZE)
{
    // PROVIDE WARNING
    std::cerr << std::endl <<
        "WARNING: Convolution reverb is still too slow to run in realtime." <<
        std::endl << std::endl;

    // Preallocate the buffers
    input_buffer = new std::complex<SAMPLE>[transform_size];
    output_buffer = new std::complex<SAMPLE>[transform_size];

    // Zero the input buffer
    for(int i=0; i < transform_size; i++)
        input_buffer[i] = 0.0;


    // Compute impulse energy to normalize
    float energy = 0.0f;
    for(int i=0; i < impulse_length; i++)
        energy += pow(in_impulse_response[i], 2);
    energy = sqrt(energy);


    // Split the impulse response into segments
    for(int i=0; i < num_impulse_blocks; i++)
    {
        // For each segment, take its FFT
        for(int j=0; j < transform_size-CONVOLUTION_BUFFER_SIZE; j++)
        {
            int t = (transform_size-CONVOLUTION_BUFFER_SIZE)*i + j;
            if(t < impulse_length)
                input_buffer[j] = in_impulse_response[t] / energy;
            else
                input_buffer[j] = 0.0;
        }

        impulse_response[i] = new std::complex<SAMPLE>[transform_size];
        transformer.fft(input_buffer, impulse_response[i]);
    }


    // Rezero the input buffer
    for(int i=0; i < transform_size; i++)
        input_buffer[i] = 0.0;


    // Initialize the output buffers
    for(int i=0; i < num_impulse_blocks; i++)
    {
        std::complex<SAMPLE>* temp = new std::complex<SAMPLE>[transform_size];
        frequency_buffer.add(temp);
    }

    for(int i=0; i < transform_size; i++)
        reverb_buffer.add(0.0);
}


ConvolutionFilter::~ConvolutionFilter()
{
    for(int i = 0; i < num_impulse_blocks; i++)
        delete impulse_response[i];
    delete input_buffer;
    delete output_buffer;
}


void ConvolutionFilter::filter(std::vector<SAMPLE>& input,
        std::vector<SAMPLE>& output, unsigned long t)
{
    // Pull in the output, return the output
    input_buffer[t % CONVOLUTION_BUFFER_SIZE] = input[0];
    output[0] = output_queue[t % CONVOLUTION_BUFFER_SIZE];

    // Call the processing if we have filled the buffer
    // Set start time of this block
    unsigned long start_t = t - CONVOLUTION_BUFFER_SIZE + 1;
    if((t+1) % CONVOLUTION_BUFFER_SIZE == 0)
        process(start_t);
}

void ConvolutionFilter::process(unsigned long start_t)
{
    // First take the FFT of the input signal
    transformer.fft(input_buffer, output_buffer);

    // Then perform each frequency multiply
    for(int i=0; i < num_impulse_blocks; i++)
    {
        // Frequency multiply
        for(int j=0; j < transform_size; j++)
        {
            frequency_buffer[start_t/CONVOLUTION_BUFFER_SIZE + i][j] +=
                output_buffer[j] * impulse_response[i][j];
        }
    }


    // Perform an inverse transform out of the frequency domain
    transformer.ifft(frequency_buffer[start_t/CONVOLUTION_BUFFER_SIZE],
                     output_buffer);
    for(int i=0; i < transform_size; i++)
        reverb_buffer[start_t + i] += output_buffer[i].real();


    // Fill the new output set
    for(int i=0; i < CONVOLUTION_BUFFER_SIZE; i++)
        output_queue[i] = reverb_buffer[start_t + i];

    
    // Update the frequency buffer
    // Zero and move the array to avoid allocating extra memory
    std::complex<SAMPLE>* temp = 
        frequency_buffer[start_t/CONVOLUTION_BUFFER_SIZE];

    for(int i=0; i < transform_size; i++)
        temp[i] = 0.0;
    frequency_buffer.add(temp);

    // Update the time buffer
    for(int i=0; i < CONVOLUTION_BUFFER_SIZE; i++)
        reverb_buffer.add(0.0);
}
