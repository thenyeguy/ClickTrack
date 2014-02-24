#include "opensles_wrapper.h"

using namespace ClickTrack;

// TODO: add logging statements to failures


OpenSlesWrapper& OpenSlesWrapper::get_instance()
{
    static OpenSlesWrapper instance;
    return instance;
}


void OpenSlesWrapper::write_outputs(std::vector< std::vector<SAMPLE> >& outputs)
{
    // Lock the buffer so that we can't write until the previous buffer is clear
    outputLock.lock();

    // Write out the next buffer
    for(unsigned i = 0; i < BUFFER_SIZE; i++)
    {
        for(unsigned j = 0; j < num_output_channels; j++)
        {
            // Automatically handle mono input
            SAMPLE sample;
            if(outputs.size() == 1)
                sample = outputs[0][i];
            else
                sample = outputs[j][i];

            // Clip instead of overflowing
            if(sample > 0.999f)  sample = 0.999f;
            if(sample <= -0.999f) sample = -0.999f;

            // Convert to signed short and save in buffer
            signed short quantized = sample * 32768;
            output_buffer[num_output_channels*i + j] = quantized;
        }
    }

    // Send the buffer to output
    handle_open_sles_error((*output_buffer_queue)->
            Enqueue(output_buffer_queue, output_buffer, num_output_channels*BUFFER_SIZE));
}


void OpenSlesWrapper::read_inputs(std::vector< std::vector<SAMPLE> >& inputs)
{
    // Lock the buffer so that we can't read until the next buffer is in
    input_lock.lock();

    // Grab the buffer from input
    handle_open_sles_error((*input_buffer_queue)->
            Enqueue(input_buffer_queue, input_buffer, num_input_channels*BUFFER_SIZE));

    // Write out the next buffer
    for(unsigned i = 0; i < BUFFER_SIZE; i++)
    {
        for(unsigned j = 0; j < inputs.size(); j++)
        {
            // Automatically handle stereo output
            short in_sample;
            if(num_input_channels == 1)
                in_sample = input_buffer[i];
            else
                in_sample = input_buffer[num_input_channels*i + j];

            // Save to output vector
            inputs[j][i] = (1.0*in_sample)/32768.0;
        }
    }
}


OpenSlesWrapper::OpenSlesWrapper()
    : num_output_channels(2), num_input_channels(1)
      //hard code mono in/stereo out
{
    // Create our buffers
    output_buffer = new OPENSLES_SAMPLE[BUFFER_SIZE*num_output_channels];
    input_buffer = new OPENSLES_SAMPLE[BUFFER_SIZE*num_input_channels];

    // Create and start the engine, using the default configuration
    handle_open_sles_error(slCreateEngine(&engine_object, 
                0, nullptr, 0, nullptr, nullptr));
    handle_open_sles_error((*engine_object)->
            Realize(engine_object, SL_BOOLEAN_FALSE));
    handle_open_sles_error((*engine_object)->
            GetInterface(engine_object, SL_IID_ENGINE, &engine));


    // Create the output mixer
    handle_open_sles_error((*engine)->
            CreateOutputMix(engine, &output_mix_object, 0, nullptr, nullptr));
    handle_open_sles_error((*output_mix_object)->
            Realize(output_mix_object, SL_BOOLEAN_FALSE));
    
    // Create the output buffer
    // First configure the data formatting
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = 
            {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format_pcm = {
        SL_DATAFORMAT_PCM,
        num_output_channels,
        SL_SAMPLINGRATE_44_1,
        SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_PCMSAMPLEFORMAT_FIXED_16,
        0, // speaker positions - 0 defaults to stereo
        SL_BYTEORDER_LITTLEENDIAN
    };
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    // Configure audio sink
    SLDataLocator_OutputMix loc_outmix = 
        {SL_DATALOCATOR_OUTPUTMIX, output_mix_object};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    // Create player
    const SLInterfaceID ids[1] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
    handle_open_sles_error((*engine)->
            CreateAudioPlayer(engine, &player_object, &audioSrc, &audioSnk, 2,
                ids, req));
    handle_open_sles_error((*player_object)->
            Realize(player_object, SL_BOOLEAN_FALSE));

    // Get the player interface and the queue out of our object
    handle_open_sles_error((*player_object)->
            GetInterface(player_object, SL_IID_PLAY, &player));
    handle_open_sles_error((*player_object)->
            GetInterface(player_object, SL_IID_BUFFERQUEUE, &output_buffer_queue));

    // Register our callback function with the queue
    handle_open_sles_error((*output_buffer_queue)->
            RegisterCallback(output_buffer_queue, output_callback, this));


    // Now configure the input system
    SLDataLocator_IODevice loc_dev = {SL_DATALOCATOR_IODEVICE,
        SL_IODEVICE_AUDIOINPUT, SL_DEFAULTDEVICEID_AUDIOINPUT, NULL};
    SLDataSource inAudioSrc = {&loc_dev, NULL};

    SLDataLocator_AndroidSimpleBufferQueue loc_bq =
        {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    format_pcm.numChannels = num_input_channels;
    SLDataSink inAudioSnk = {&loc_bq, &format_pcm};

    // Create input device
    handle_open_sles_error((*engine)->
            CreateAudioRecorder(engine, &recorder_object, &inAudioSrc, &inAudioSnk, 
                1, ids, req));
    handle_open_sles_error((*recorder_object)->
            Realize(recorder_object, SL_BOOLEAN_FALSE));
    handle_open_sles_error((*recorder_object)->
            GetInterface(recorder_object, SL_IID_RECORD, &recorder));

    // Register our input callback
    handle_open_sles_error((*input_buffer_queue)->RegisterCallback(
                input_buffer_queue, input_callback, &input_buffer_queue));


    // Begin playing
    handle_open_sles_error((*player)->
            SetPlayState(player, SL_PLAYSTATE_PLAYING));
    handle_open_sles_error((*recorder)->
        SetRecordState(recorder,SL_RECORDSTATE_RECORDING));
}


OpenSlesWrapper::~OpenSlesWrapper()
{
    // Release the player
    (*player_object)->Destroy(player_object);

    // Release the output mixer
    (*output_mix_object)->Destroy(output_mix_object);

    // Release the engine
    (*engine_object)->Destroy(engine_object);

    // Free the output buffer
    delete output_buffer;
}


void OpenSlesWrapper::output_callback(SLAndroidSimpleBufferQueueItf bq,
        void* context)
{
    // Unlock the buffer so that we can write the next buffer in
    OpenSlesWrapper* obj = (OpenSlesWrapper*) context;
    obj->outputLock.unlock();
}


void OpenSlesWrapper::input_callback(SLAndroidSimpleBufferQueueItf bq,
        void* context)
{
    // Unlock the buffer so that we can write the next buffer in
    OpenSlesWrapper* obj = (OpenSlesWrapper*) context;
    obj->input_lock.unlock();
}


void OpenSlesWrapper::handle_open_sles_error(SLresult result)
{
    if(result == SL_RESULT_SUCCESS)
        return;

    // Immediately die
    exit(1);
}
