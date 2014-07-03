#ifndef MIDI_GENERICS_H
#define MIDI_GENERICS_H

#include <vector>


namespace ClickTrack
{
    /* The following message packet defines a standard MIDI protocol. A message
     * is broken into its message type, the channel it was sent to, and the
     * remaining (unprocessed) raw bytes of the message
     */
    enum MidiMessageType
    {
        NOTE_UP = 0x8,
        NOTE_DOWN = 0x9,
        POLYPHONIC_KEY_PRESSURE = 0xA,
        CONTROL_CHANGE = 0xB,
        PROGRAM_CHANGE = 0xC,
        CHANNEL_PRESSURE = 0xD,
        PITCH_BEND = 0xE,
        SYSTEM_MESSAGE = 0xF
    };

    struct MidiMessage
    {
        MidiMessageType type;
        unsigned char channel;
        std::vector<unsigned char> message;
    };


    /* Converts a MIDI note number to a frequency
     */
    float midiNoteToFreq(unsigned note);


    /* An output channel is the basic unit with which an object receives events.
     * It is contained within an MidiGenerator object, and serves to pipe events
     * from its parent generator into a buffer that a later element can access.
     *
     * Contains boilerplate code to lazily update its output buffer when
     * requested.
     */
    class MidiGenerator;
    class MidiChannel
    {
        friend class MidiGenerator;
        friend class MidiFilter;

        public:
            /* Fills an incoming buffer with one block worth of audio data
             * beginning at the requested time.
             */
            std::vector<MidiMessage> get_events(unsigned long t);

        private:
            /* A channel can only exist within an audio generator, so protect
             * the constructor
             */
            MidiChannel(MidiGenerator& in_parent, unsigned long start_t=0);

            /* Called by the midi generator, this registers the next time
             * step's output events
             */
            void push_events(std::vector<MidiMessage>& events);

            /* Internal state
             */
            MidiGenerator& parent;
            std::vector<MidiMessage> last_events;
            unsigned long next_time;
    };


    /* An midi generator is a basic event chain element. It must have the
     * ability to write out event data into an output channel.
     */
    class MidiGenerator
    {
        friend class MidiChannel;
        friend class MidiFilter;

        public:
            MidiGenerator();
            virtual ~MidiGenerator() {}

            /* Getter for output channel
             */
            MidiChannel* get_output_midi_channel();

        private:
            /* Writes outputs into the buffer. Calls tick to determine what to
             * write out. Used by the output channel
             */
            virtual void tick(unsigned long t);

            /* When called, updates the output channels with one frame of
             * events at time t.
             *
             * Must be overwritten in subclasses.
             */
            virtual void generate_events(std::vector<MidiMessage>& outputs, 
                    unsigned long t) = 0;

            /* Information about our internal output channels
             */
            MidiChannel output_channel;
            std::vector<MidiMessage> output_frame;
    };


    /* A midi consumer is a basic event chain element. It must have the
     * ability to read in event data from an output channel, and perform an
     * operation on that input data.
     */
    class MidiConsumer
    {
        friend class TimingManager;
        friend class MidiFilter;

        public:
            MidiConsumer();
            virtual ~MidiConsumer() {}

            /* Funtions to connect and disconnect channels.
             */
            void set_input_midi_channel(MidiChannel* channel);
            void remove_channel();

        private:
            /* When called, reads in the next frame from the input channels
             * and calls the tick function.
             */
            virtual void tick(unsigned long t);

            /* When called on input data, processes it. Must be overwritten in
             * subclass.
             */
            virtual void process_events(std::vector<MidiMessage>& inputs, 
                    unsigned long t) = 0;

            /* Information about our internal input channels
             */
            MidiChannel* input_channel;
            std::vector<MidiMessage> input_frame;
    };


    /* The MidiFilter is a hybrid event chain element. It both consumes and
     * generates event data, and acts as a pure function that transforms input
     * events to output events.
     */
    class MidiFilter : public MidiGenerator, public MidiConsumer
    {
        public:
            MidiFilter();
            virtual ~MidiFilter() {}

        private:
            /* When called, reads in the next frame from the input channels,
             * processes it and write to the output channels.
             */
            void tick(unsigned long t);

            /* Given an input frame, generate a frame of output data. Must be
             * overwritten in subclass.
             */
            virtual void filter_events(std::vector<MidiMessage>& inputs, 
                    std::vector<MidiMessage>& outputs, unsigned long t) = 0;

        private:
            /* To properly implement the tick override, these functions must be
             * defined. They do nothing.
             */
            void generate_events(std::vector<MidiMessage>& inputs, unsigned long t) {}
            void process_events(std::vector<MidiMessage>& outputs, unsigned long t) {}
    };
}

#endif
