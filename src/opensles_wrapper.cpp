#include "logcat.h"
#include "opensles_wrapper.h"

using namespace ClickTrack;


OpenSlesWrapper& OpenSlesWrapper::get_instance()
{
    static OpenSlesWrapper instance;
    return instance;
}


void OpenSlesWrapper::write_outputs(std::vector< std::vector<SAMPLE> >& outputs)
{
    // Write out the next buffer
    for(unsigned i = 0; i < BUFFER_SIZE; i++)
    {
        for(unsigned j = 0; j < num_channels; j++)
        {
            // Automatically handle mono input
            SAMPLE sample;
            if(outputs.size() == 1)
                sample = outputs[0][i];
            else
                sample = outputs[j][i];

            // Clip instead of overflowing
            if(sample > 0.999f)  sample = 0.999f;
            if(sample < -0.999f) sample = -0.999f;

            // Convert and save in buffer
            output_buffer[num_channels*i + j] = sample * 32768;
        }
    }

    // Send the buffer to output
    output_lock.lock();
    check_error("output_buffer_queue->Enqueue", (*output_buffer_queue)->
            Enqueue(output_buffer_queue, output_buffer, 
                num_channels*BUFFER_SIZE*sizeof(OPENSLES_SAMPLE)));
}


void OpenSlesWrapper::read_inputs(std::vector< std::vector<SAMPLE> >& inputs)
{
    // Grab the buffer from input
    input_lock.lock();
    check_error("input_buffer_queue->Enqueue", (*input_buffer_queue)->
            Enqueue(input_buffer_queue, input_buffer, 
                num_channels*BUFFER_SIZE*sizeof(OPENSLES_SAMPLE)));


    // Write out the next buffer
    for(unsigned i = 0; i < BUFFER_SIZE; i++)
    {
        for(unsigned j = 0; j < inputs.size(); j++)
        {
            // Automatically handle stereo output
            OPENSLES_SAMPLE in_sample;
            if(num_channels == 1)
                in_sample = input_buffer[i];
            else
                in_sample = input_buffer[num_channels*i + j];

            // Save to output vector
            inputs[j][i] = in_sample/32768.0;
        }
    }
}


OpenSlesWrapper::OpenSlesWrapper()
    : num_channels(1)
      //hard code mono in/stereo out
{
    logi("Initializing OpenSL ES wrapper\n");

    // Create our buffers
    output_buffer = new OPENSLES_SAMPLE[BUFFER_SIZE*num_channels];
    input_buffer = new OPENSLES_SAMPLE[BUFFER_SIZE*num_channels];


    // Define our audio format
    SLDataFormat_PCM format_pcm = {
        SL_DATAFORMAT_PCM,
        num_channels,
        SL_SAMPLINGRATE_44_1,
        SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_SPEAKER_FRONT_CENTER,
        SL_BYTEORDER_LITTLEENDIAN
    };


    // Create and start the engine, using the default configuration
    check_error("slCreateEngine", slCreateEngine(&engine_object, 
                0, nullptr, 0, nullptr, nullptr));
    check_error("engine->Realize", (*engine_object)->
            Realize(engine_object, SL_BOOLEAN_FALSE));
    check_error("engine->GetInterface", (*engine_object)->
            GetInterface(engine_object, SL_IID_ENGINE, &engine));


    // Create the output mixer
    check_error("CreateOutputMix", (*engine)->
            CreateOutputMix(engine, &output_mix_object, 0, nullptr, nullptr));
    check_error("output_mix->Realize", (*output_mix_object)->
            Realize(output_mix_object, SL_BOOLEAN_FALSE));
    

    // Create the output player
    // First configure the audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = 
            {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataSource outputSrc = {&loc_bufq, &format_pcm};

    // Configure audio sink
    SLDataLocator_OutputMix loc_outmix = 
        {SL_DATALOCATOR_OUTPUTMIX, output_mix_object};
    SLDataSink outputSink = {&loc_outmix, NULL};

    // Create the player
    const SLInterfaceID ids[1] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
    check_error("CreateAudioPlayer", (*engine)->
            CreateAudioPlayer(engine, &player_object, &outputSrc, &outputSink, 1,
                ids, req));
    check_error("player->Realize", (*player_object)->
            Realize(player_object, SL_BOOLEAN_FALSE));
    check_error("player->GetInterface", (*player_object)->
            GetInterface(player_object, SL_IID_PLAY, &player));


    // Configure the output buffer
    check_error("player->GetInterface(output_buffer_queue)", (*player_object)->
            GetInterface(player_object, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, 
                &output_buffer_queue));
    check_error("output_buffer_queue->RegisterCallback", (*output_buffer_queue)->
            RegisterCallback(output_buffer_queue, output_callback, this));
    check_error("output_buffer_queue->Clear", (*output_buffer_queue)->
            Clear(output_buffer_queue) );


    // Now configure the input system
    SLDataLocator_IODevice loc_dev = {SL_DATALOCATOR_IODEVICE,
        SL_IODEVICE_AUDIOINPUT, SL_DEFAULTDEVICEID_AUDIOINPUT, NULL};
    SLDataSource inAudioSrc = {&loc_dev, NULL};

    SLDataLocator_AndroidSimpleBufferQueue loc_bq =
        {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataSink inAudioSnk = {&loc_bq, &format_pcm};

    // Create input device
    check_error("CreateAudioRecorder", (*engine)->
            CreateAudioRecorder(engine, &recorder_object, &inAudioSrc, &inAudioSnk, 
                1, ids, req));
    check_error("recorder->Realize", (*recorder_object)->
            Realize(recorder_object, SL_BOOLEAN_FALSE));
    check_error("recorder->GetInterface", (*recorder_object)->
            GetInterface(recorder_object, SL_IID_RECORD, &recorder));


    // Configure the input buffer
    check_error("recorder->GetInterface(input_buffer_queue)", (*recorder_object)->
            GetInterface(recorder_object, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, 
                &input_buffer_queue));
    check_error("output_buffer_queue->RegisterCallback", (*input_buffer_queue)->
            RegisterCallback(input_buffer_queue, input_callback, 
                this));
    check_error("input_buffer_queue->Clear", (*input_buffer_queue)->
            Clear(input_buffer_queue) );


    // Begin playing
    check_error("player->SetPlayState(PLAYING)", (*player)->
            SetPlayState(player, SL_PLAYSTATE_PLAYING));
    check_error("recorder->SetRecordState(RECORDING)", (*recorder)->
        SetRecordState(recorder,SL_RECORDSTATE_RECORDING));

    logi("Successfully initialized OpenSL ES wrapper\n");
}


OpenSlesWrapper::~OpenSlesWrapper()
{
    logi("Destroying OpenSL ES wrapper\n");

    // Release the player
    (*player_object)->Destroy(player_object);

    // Release the output mixer
    (*output_mix_object)->Destroy(output_mix_object);

    // Release the engine
    (*engine_object)->Destroy(engine_object);

    // Free the output buffer
    delete output_buffer;

    logi("Successfully destroyed OpenSL ES wrapper\n");
}


void OpenSlesWrapper::output_callback(SLAndroidSimpleBufferQueueItf bq,
        void* context)
{
    // Unlock the buffer so that we can write the next buffer in
    OpenSlesWrapper* obj = (OpenSlesWrapper*) context;
    obj->output_lock.unlock();
}


void OpenSlesWrapper::input_callback(SLAndroidSimpleBufferQueueItf bq,
        void* context)
{
    // Unlock the buffer so that we can write the next buffer in
    OpenSlesWrapper* obj = (OpenSlesWrapper*) context;
    obj->input_lock.unlock();
}


void OpenSlesWrapper::check_error(const char* info, SLresult result)
{
    if(result == SL_RESULT_SUCCESS)
        return;

    // Convert error to string
    const char* error_string;
    switch(result)
    {
        case SL_RESULT_SUCCESS:
            error_string = "SL_RESULT_SUCCESS";
        case SL_RESULT_PRECONDITIONS_VIOLATED:
            error_string = "SL_RESULT_PRECONDITIONS_VIOLATED";
            break;
        case SL_RESULT_PARAMETER_INVALID:
            error_string = "SL_RESULT_PARAMETER_INVALID";
            break;
        case SL_RESULT_MEMORY_FAILURE:
            error_string = "SL_RESULT_MEMORY_FAILURE";
            break;
        case SL_RESULT_RESOURCE_ERROR:
            error_string = "SL_RESULT_RESOURCE_ERROR";
            break;
        case SL_RESULT_RESOURCE_LOST:
            error_string = "SL_RESULT_RESOURCE_LOST";
            break;
        case SL_RESULT_IO_ERROR:
            error_string = "SL_RESULT_IO_ERROR";
            break;
        case SL_RESULT_BUFFER_INSUFFICIENT:
            error_string = "SL_RESULT_BUFFER_INSUFFICIENT";
            break;
        case SL_RESULT_CONTENT_CORRUPTED:
            error_string = "SL_RESULT_CONTENT_CORRUPTED";
            break;
        case SL_RESULT_CONTENT_UNSUPPORTED:
            error_string = "SL_RESULT_CONTENT_UNSUPPORTED";
            break;
        case SL_RESULT_CONTENT_NOT_FOUND:
            error_string = "SL_RESULT_CONTENT_NOT_FOUND";
            break;
        case SL_RESULT_PERMISSION_DENIED:
            error_string = "SL_RESULT_PERMISSION_DENIED";
            break;
        case SL_RESULT_FEATURE_UNSUPPORTED:
            error_string = "SL_RESULT_FEATURE_UNSUPPORTED";
            break;
        case SL_RESULT_INTERNAL_ERROR:
            error_string = "SL_RESULT_INTERNAL_ERROR";
            break;
        case SL_RESULT_UNKNOWN_ERROR:
            error_string = "SL_RESULT_UNKNOWN_ERROR";
            break;
        case SL_RESULT_OPERATION_ABORTED:
            error_string = "SL_RESULT_OPERATION_ABORTED";
            break;
        case SL_RESULT_CONTROL_LOST:
            error_string = "SL_RESULT_CONTROL_LOST";
            break;
        default:
            error_string = "UNKNOWN ERROR";
    }

    // Log error and die
    loge("Received OpenSL ES error (%s): %s\n", info, error_string);
    exit(1);
}
