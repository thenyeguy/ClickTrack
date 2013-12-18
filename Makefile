#Define compiler
CC      = clang++
CFLAGS  = -I/usr/local/include -Wall -Werror -g 
LIBS    = -L/usr/local/lib -lportaudio

#Define compile paths
SRCDIR  = src
TSTDIR  = tst
BINDIR  = bin
OBJDIR  = obj
vpath %.cpp $(SRCDIR)

# Primary target
all: tests
full: clean all

tests: test_ringbuffer test_fft test_convolve test_filterchain test_wav


# Macro the filter list
IOCORE_SRC = portaudio_wrapper.cpp fft.cpp filter_generics.cpp \
             io_elements.cpp oscillator.cpp
FILTER_SRC = elementary_filters.cpp convolve.cpp delay.cpp reverb.cpp


#Define test target and dependencides
TEST_RINGBUFFER_SRC = test_ringbuffer.cpp
TEST_RINGBUFFER_OBJ = $(addprefix $(OBJDIR)/, $(TEST_RINGBUFFER_SRC:.cpp=.o))
test_ringbuffer: $(TEST_RINGBUFFER_OBJ) $(BINDIR)
	@echo "Linking $(BINDIR)/$@...\n"
	@$(CC) $(CFLAGS) $(LIBS) $(TEST_RINGBUFFER_OBJ) -o $(BINDIR)/$@


TEST_FFT_SRC = fft.cpp test_fft.cpp
TEST_FFT_OBJ = $(addprefix $(OBJDIR)/, $(TEST_FFT_SRC:.cpp=.o))
test_fft: $(TEST_FFT_OBJ) $(BINDIR)
	@echo "Linking $(BINDIR)/$@...\n"
	@$(CC) $(CFLAGS) $(LIBS) $(TEST_FFT_OBJ) -o $(BINDIR)/$@


TEST_CONVOLVE_SRC = $(IOCORE_SRC) $(FILTER_SRC) test_convolve.cpp
TEST_CONVOLVE_OBJ = $(addprefix $(OBJDIR)/, $(TEST_CONVOLVE_SRC:.cpp=.o))
test_convolve: $(TEST_CONVOLVE_OBJ) $(BINDIR)
	@echo "Linking $(BINDIR)/$@...\n"
	@$(CC) $(CFLAGS) $(LIBS) $(TEST_CONVOLVE_OBJ) -o $(BINDIR)/$@


TEST_FILTERCHAIN_SRC = $(IOCORE_SRC) $(FILTER_SRC) test_filterchain.cpp
TEST_FILTERCHAIN_OBJ = $(addprefix $(OBJDIR)/, $(TEST_FILTERCHAIN_SRC:.cpp=.o))
test_filterchain: $(TEST_FILTERCHAIN_OBJ) $(BINDIR)
	@echo "Linking $(BINDIR)/$@...\n"
	@$(CC) $(CFLAGS) $(LIBS) $(TEST_FILTERCHAIN_OBJ) -o $(BINDIR)/$@


TEST_WAV_SRC = $(IOCORE_SRC) test_wav.cpp
TEST_WAV_OBJ = $(addprefix $(OBJDIR)/, $(TEST_WAV_SRC:.cpp=.o))
test_wav: $(TEST_WAV_OBJ) $(BINDIR)
	@echo "Linking $(BINDIR)/$@...\n"
	@$(CC) $(CFLAGS) $(LIBS) $(TEST_WAV_OBJ) -o $(BINDIR)/$@


#Define helper macros
$(OBJDIR)/%.o: %.cpp $(OBJDIR)
	@echo "Compiling $<"
	@$(CC) -c $(CFLAGS) $< -o $@
	
$(BINDIR):
	@mkdir $(BINDIR)
	
$(OBJDIR):
	@mkdir $(OBJDIR)

clean:
	@echo "Cleaning...\n"
	@rm -rf $(BINDIR)
	@rm -rf $(OBJDIR)
