#include <stdio.h>
#include <sndfile.h>
#include <iostream>
#include "portaudio.h"
#include <malloc.h>
#include <wx/wx.h>
#include <wx/sizer.h>
#include <pthread.h>
#include "Visualizer.h"
using namespace std;

// Prototypes
int playAudio(const char *filePath);
int playFrames();
void *initAudio(void *ptr);
void *initGUI(void *ptr);

// Shared data structures
static float* frames = NULL;
static VisApp* gui;

struct args {
	int argc;
	char **argv;
};

int main(int argc, char *argv[]) {
	// unused parameters
	(void) argc;
	(void) argv;

	// initialize threads
	pthread_t audioThread, guiThread;
	int audRet, guiRet;
	args init;
	init.argc = argc;
	init.argv = argv;
	gui = new VisApp();

	// create threads
	audRet = pthread_create( &audioThread, NULL, initAudio, NULL);
	guiRet = pthread_create( &guiThread, NULL, initGUI, (void *) &init);

	// wait for threads to be done
	pthread_join( audioThread, NULL );
	pthread_join( guiThread, NULL );

	// return result
	return (audRet && guiRet);
}

void *initAudio(void *ptr) {
	(void) ptr;

	playAudio("TRACK001.WAV");
	return NULL;
}

void *initGUI(void *arguments) {
	args *init;
	init = (args *) arguments;

	wxApp::SetInstance(gui);
	wxEntryStart( init->argc, init->argv );
	wxTheApp->OnInit();
	wxTheApp->OnRun();
	//wxTheApp->OnExit();
	//wxEntryCleanup();

    return NULL;
	/*
	args *init = (args *) ptr;
	QApplication a(init->argc, init->argv);

	a.exec();
	return NULL;
	*/
}

int playAudio(const char *filePath) {
	// Open sound file
	SF_INFO sndInfo;
	SNDFILE *sndFile = sf_open(filePath, SFM_READ, &sndInfo);
	if (sndFile == NULL) {
		fprintf(stderr, "Error reading source file '%s': %s\n", filePath, sf_strerror(sndFile));
		return 1;
	}

	// Check format - 16bit PCM
	if (sndInfo.format != (SF_FORMAT_WAV | SF_FORMAT_PCM_16)) {
		fprintf(stderr, "Input should be 16bit Wav\n");
		sf_close(sndFile);
		return 1;
	}

	// Check channels - mono
	if (sndInfo.channels != 1) {
		fprintf(stderr, "Wrong number of channels\n");
		sf_close(sndFile);
		return 1;
	}

	// Allocate memory
	frames = (float*) malloc(sndInfo.frames * sizeof(float));
	float* start = frames;
	gui->setFramePtr(&frames);
	if (frames == NULL) {
		fprintf(stderr, "Could not allocate memory for file\n");
		sf_close(sndFile);
		return 1;
	}

	// Load data
	long numFrames = sf_readf_float(sndFile, frames, sndInfo.frames);
	/*
	//Examining samples for the lulz
	int i = 0;
	while (i < 44100) {
		printf("Frame %d: %f\n", i, *(frames + i));
		i++;
	}
	*/
	

	// Check correct number of samples loaded
	if (numFrames != sndInfo.frames) {
		fprintf(stderr, "Did not read enough frames for source\n");
		sf_close(sndFile);
		free(frames);
		return 1;
	}

	playFrames();

	// Output Info
	printf("Read %ld frames from %s, Sample rate: %d, Length: %fs\n",
		numFrames, filePath, sndInfo.samplerate, (float)numFrames/sndInfo.samplerate);
	
	sf_close(sndFile);
	//free(start);

	return 0;
}

static int patestCallback( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo *timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData ) {
    /* Cast data passed through stream to our structure. */
    float *out = (float*)outputBuffer;
    unsigned int i;
    (void) userData;
    (void) inputBuffer; /* Prevent unused variable warning. */
	(void) timeInfo;
	(void) statusFlags;

    for( i=0; i<framesPerBuffer; i++ )
    {
		*out++ = *frames++;
    }
    return 0;
}

int playFrames() {
	PaStream *stream;
	PaError err;

    printf("PortAudio Test: output Abhay sounds.\n");

    /* Initialize library before making any other calls. */
    err = Pa_Initialize();
    if( err != paNoError ) goto error;
	
    /* Open an audio I/O stream. */
    err = Pa_OpenDefaultStream( &stream,
                                0,          /* no input channels */
                                1,          /* mono output */
                                paFloat32,  /* 32 bit floating point output */
                                44100,
                                256,        /* frames per buffer */
                                patestCallback,
                                NULL );
    if( err != paNoError ) goto error;

    err = Pa_StartStream( stream );
    if( err != paNoError ) goto error;

    /* Sleep for several seconds. */
    Pa_Sleep(180000);

    err = Pa_StopStream( stream );
    if( err != paNoError ) goto error;
    err = Pa_CloseStream( stream );
    if( err != paNoError ) goto error;
    Pa_Terminate();
    printf("Test finished.\n");
    return err;
error:
    Pa_Terminate();
    fprintf( stderr, "An error occured while using the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", err );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
    return err;
}
