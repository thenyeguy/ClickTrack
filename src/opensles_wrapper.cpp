#include "opensles_wrapper.h"

using namespace ClickTrack;

// TODO: add logging statements to failures


OpenSlesWrapper& OpenSlesWrapper::getInstance()
{
    static OpenSlesWrapper instance;
    return instance;
}


void OpenSlesWrapper::writeOutputs(std::vector< std::vector<SAMPLE> >& outputs)
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
            outputBuffer[num_output_channels*i + j] = quantized;
        }
    }

    // Send the buffer to output
    handleOpenSlesError((*outputBufferQueue)->
            Enqueue(outputBufferQueue, outputBuffer, num_output_channels*BUFFER_SIZE));
}


void OpenSlesWrapper::readInputs(std::vector< std::vector<SAMPLE> >& inputs)
{
    // Lock the buffer so that we can't read until the next buffer is in
    inputLock.lock();

    // Grab the buffer from input
    handleOpenSlesError((*inputBufferQueue)->
            Enqueue(inputBufferQueue, inputBuffer, num_input_channels*BUFFER_SIZE));

    // Write out the next buffer
    for(unsigned i = 0; i < BUFFER_SIZE; i++)
    {
        for(unsigned j = 0; j < inputs.size(); j++)
        {
            // Automatically handle stereo output
            short in_sample;
            if(num_input_channels == 1)
                in_sample = inputBuffer[i];
            else
                in_sample = inputBuffer[num_input_channels*i + j];

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
    outputBuffer = new OPENSLES_SAMPLE[BUFFER_SIZE*num_output_channels];
    inputBuffer = new OPENSLES_SAMPLE[BUFFER_SIZE*num_input_channels];

    // Create and start the engine, using the default configuration
    handleOpenSlesError(slCreateEngine(&engineObject, 
                0, nullptr, 0, nullptr, nullptr));
    handleOpenSlesError((*engineObject)->
            Realize(engineObject, SL_BOOLEAN_FALSE));
    handleOpenSlesError((*engineObject)->
            GetInterface(engineObject, SL_IID_ENGINE, &engine));


    // Create the output mixer
    handleOpenSlesError((*engine)->
            CreateOutputMix(engine, &outputMixObject, 0, nullptr, nullptr));
    handleOpenSlesError((*outputMixObject)->
            Realize(outputMixObject, SL_BOOLEAN_FALSE));
    
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
        {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    // Create player
    const SLInterfaceID ids[1] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
    handleOpenSlesError((*engine)->
            CreateAudioPlayer(engine, &playerObject, &audioSrc, &audioSnk, 2,
                ids, req));
    handleOpenSlesError((*playerObject)->
            Realize(playerObject, SL_BOOLEAN_FALSE));

    // Get the player interface and the queue out of our object
    handleOpenSlesError((*playerObject)->
            GetInterface(playerObject, SL_IID_PLAY, &player));
    handleOpenSlesError((*playerObject)->
            GetInterface(playerObject, SL_IID_BUFFERQUEUE, &outputBufferQueue));

    // Register our callback function with the queue
    handleOpenSlesError((*outputBufferQueue)->
            RegisterCallback(outputBufferQueue, outputCallback, this));


    // Now configure the input system
    SLDataLocator_IODevice loc_dev = {SL_DATALOCATOR_IODEVICE,
        SL_IODEVICE_AUDIOINPUT, SL_DEFAULTDEVICEID_AUDIOINPUT, NULL};
    SLDataSource inAudioSrc = {&loc_dev, NULL};

    SLDataLocator_AndroidSimpleBufferQueue loc_bq =
        {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    format_pcm.numChannels = num_input_channels;
    SLDataSink inAudioSnk = {&loc_bq, &format_pcm};

    // Create input device
    handleOpenSlesError((*engine)->
            CreateAudioRecorder(engine, &recorderObject, &inAudioSrc, &inAudioSnk, 
                1, ids, req));
    handleOpenSlesError((*recorderObject)->
            Realize(recorderObject, SL_BOOLEAN_FALSE));
    handleOpenSlesError((*recorderObject)->
            GetInterface(recorderObject, SL_IID_RECORD, &recorder));

    // Register our input callback
    handleOpenSlesError((*inputBufferQueue)->RegisterCallback(
                inputBufferQueue, inputCallback, &inputBufferQueue));


    // Begin playing
    handleOpenSlesError((*player)->
            SetPlayState(player, SL_PLAYSTATE_PLAYING));
    handleOpenSlesError((*recorder)->
        SetRecordState(recorder,SL_RECORDSTATE_RECORDING));
}


OpenSlesWrapper::~OpenSlesWrapper()
{
    // Release the player
    (*playerObject)->Destroy(playerObject);

    // Release the output mixer
    (*outputMixObject)->Destroy(outputMixObject);

    // Release the engine
    (*engineObject)->Destroy(engineObject);

    // Free the output buffer
    delete outputBuffer;
}


void OpenSlesWrapper::outputCallback(SLAndroidSimpleBufferQueueItf bq,
        void* context)
{
    // Unlock the buffer so that we can write the next buffer in
    OpenSlesWrapper* obj = (OpenSlesWrapper*) context;
    obj->outputLock.unlock();
}


void OpenSlesWrapper::inputCallback(SLAndroidSimpleBufferQueueItf bq,
        void* context)
{
    // Unlock the buffer so that we can write the next buffer in
    OpenSlesWrapper* obj = (OpenSlesWrapper*) context;
    obj->inputLock.unlock();
}


void OpenSlesWrapper::handleOpenSlesError(SLresult result)
{
    if(result == SL_RESULT_SUCCESS)
        return;

    // Immediately die
    exit(1);
}
