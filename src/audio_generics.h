#ifndef AUDIO_GENERICS_H
#define AUDIO_GENERICS_H

#include <vector>
#include "portaudio_wrapper.h"


namespace ClickTrack
{
    /* This determines the size of our internal ring buffers.
     */
    const unsigned DEFAULT_RINGBUFFER_SIZE = BUFFER_SIZE;


    /* An output channel is the basic unit with which an object receives audio.
     * It is contained within an AudioGenerator object, and serves to pipe audio
     * from its parent generator into a buffer that a later element can access.
     *
     * Contains boilerplate code to lazily update its output buffer when
     * requested.
     */
    class AudioGenerator;
    class AudioConsumer;
    class Channel
    {
        friend class AudioGenerator;
        friend class AudioConsumer;
        friend class AudioFilter;

        public:
            /* Fills an incoming buffer with one block worth of audio data
             * beginning at the requested time.
             */
            SAMPLE get_sample(unsigned long t);

        protected:
            /* A channel can only exist within an audio generator, so protect
             * the constructor
             */
            Channel(AudioGenerator& in_parent, unsigned long start_t=0);

            void push_sample(SAMPLE s);

            /* Internal state
             */
            AudioGenerator& parent;
            SAMPLE last_sample;
            unsigned long next_time;
    };


    /* An audio generator is a basic signal chain element. It must have the
     * ability to write out audio data into an output channel.
     *
     * EG a microphone is a generator.
     */
    class AudioGenerator
    {
        friend class Channel;

        public:
            AudioGenerator(unsigned num_output_channels = 1);
            virtual ~AudioGenerator() {}

            /* Getters for output channels
             */
            unsigned get_num_output_channels();
            Channel* get_output_channel(unsigned i = 0);

        protected:
            /* When called, updates the output channels with one more frame of
             * audio at time t.
             *
             * Must be overwritten in subclasses.
             */
            virtual void generate_outputs(std::vector<SAMPLE>& outputs, 
                    unsigned long t) = 0;

            /* Writes outputs into the buffer. Calls tick to determine what to
             * write out. Used by the output channel
             */
            virtual void tick(unsigned long t);

            /* Information about our internal output channels
             */
            std::vector<Channel> output_channels;
            std::vector<SAMPLE> output_frame;
    };


    /* An audio consumer is a basic signal chain element. It must have the
     * ability to read in audio data from an output channel, and perform an
     * operation on that input data.
     *
     * A consumer can optionally trigger a callback function whenever it
     * consumes its outputs.
     *
     * EG a speaker is a consumer.
     */
    class AudioConsumer
    {
        friend class TimingManager;

        public:
            AudioConsumer(unsigned num_input_channels = 1);
            virtual ~AudioConsumer() {}

            /* Funtions to connect and disconnect channels. You can also look up
             * a channel's index by value, so that it can be removed and
             * restored when switching components
             */
            unsigned get_num_input_channels();
            void set_input_channel(Channel* channel, unsigned channel_i = 0);
            void remove_channel(unsigned channel_i);

            unsigned get_channel_index(Channel* channel);

        protected:
            /* When called, reads in the next frame from the input channels
             * and calls the tick function.
             */
            virtual void tick(unsigned long t);

            /* When called on input data, processes it. Must be overwritten in
             * subclass.
             */
            virtual void process_inputs(std::vector<SAMPLE>& inputs, 
                    unsigned long t) = 0;

            /* Information about our internal input channels
             */
            std::vector<Channel*> input_channels;
            std::vector<SAMPLE> input_frame;
    };


    /* The AudioFilter is a hybrid signal chain element. It both consumes and
     * generates audio data, and acts as a pure function that transforms input
     * audio to output audio.
     *
     * EG a reverb is a filter
     */
    class AudioFilter : public AudioGenerator, public AudioConsumer
    {
        public:
            AudioFilter(unsigned num_input_channels = 1,
                    unsigned num_output_channels = 1);
            virtual ~AudioFilter() {}

        protected:
            /* Given an input frame, generate a frame of output data. Must be
             * overwritten in subclass.
             */
            virtual void filter(std::vector<SAMPLE>& input, 
                    std::vector<SAMPLE>& output, unsigned long t) = 0;

            /* When called, reads in the next frame from the input channels
             * and write to the output channels.
             */
            void tick(unsigned long t);

        private:
            /* To properly implement the tick override, these functions must be
             * defined. They do nothing.
             */
            void generate_outputs(std::vector<SAMPLE>& inputs, unsigned long t);
            void process_inputs(std::vector<SAMPLE>& outputs, unsigned long t);

            /* Statically allocated frame for speed. Seperate from the
             * generator's output to maintain clean interface
             */
            std::vector<SAMPLE> output_frame;
    };


    /* Exceptions used by the audio generics
     */
    class ChannelOutOfRange: public std::exception
    {
        virtual const char* what() const throw()
        {
            return "The requested filter does not have this many output channels.";
        }
    };
    class NoEmptyInputChannel: public std::exception
    {
        virtual const char* what() const throw()
        {
            return "This filter cannot accept further input channels.";
        }
    };
    class ChannelNotFound: public std::exception
    {
        virtual const char* what() const throw()
        {
            return "This filter does not contained the specified input channel.";
        }
    };
}

#endif
