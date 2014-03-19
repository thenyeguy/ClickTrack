#include <iostream>
#include <complex>
#include "convolve.h"

using namespace ClickTrack;


/*
ConvolutionFilter::ConvolutionFilter(unsigned impulse_length,
                                     SAMPLE* in_impulse_response)
    : AudioFilter(1),

      transform_size(fft_multiplier*FRAME_SIZE),
      transformer(transform_size),

      num_impulse_blocks((impulse_length - 1) / 
                         (transform_size-FRAME_SIZE) + 1),
        // shift by one so perfect powers of two don't get overallocated
      impulse_response(num_impulse_blocks, NULL),

      frequency_buffer(num_impulse_blocks),
      reverb_buffer(transform_size)
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
        for(int j=0; j < transform_size-FRAME_SIZE; j++)
        {
            int t = (transform_size-FRAME_SIZE)*i + j;
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
    // First take the FFT of the input signal
    for(int i = 0; i < FRAME_SIZE; i++)
        input_buffer[i] = input[0][i];
    transformer.fft(input_buffer, output_buffer);


    // Then perform each frequency multiply
    for(int i=0; i < num_impulse_blocks; i++)
    {
        // Frequency multiply
        for(int j=0; j < transform_size; j++)
        {
            frequency_buffer[next_t/FRAME_SIZE + i][j] +=
                output_buffer[j] * impulse_response[i][j];
        }
    }


    // Perform an inverse transform out of the frequency domain
    transformer.ifft(frequency_buffer[next_t/FRAME_SIZE],
                     output_buffer);
    for(int i=0; i < transform_size; i++)
        reverb_buffer[next_t + i] += output_buffer[i].real();


    // Extract the new output set
    for(int i=0; i < FRAME_SIZE; i++)
        output[0][i] = reverb_buffer[next_t + i];

    
    // Update the frequency buffer
    // Zero and move the array to avoid allocating extra memory
    std::complex<SAMPLE>* temp = frequency_buffer[next_t/FRAME_SIZE];
    for(int i=0; i < transform_size; i++)
        temp[i] = 0.0;
    frequency_buffer.add(temp);

    // Update the time buffer
    for(int i=0; i < FRAME_SIZE; i++)
        reverb_buffer.add(0.0);
}
*/
