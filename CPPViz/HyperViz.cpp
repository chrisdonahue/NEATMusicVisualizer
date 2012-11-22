#include <sndfile.h>
#include "portaudio.h"

#include <iostream>
#include <stdio.h>
#include <malloc.h>

#include <wx/wx.h>
#include <wx/sizer.h>

#include <pthread.h>

#include "Visualizer.h"

#include "NEAT/neat.h"
#include "NEAT/population.h"

using namespace std;

// THREAD INIT METHOD PROTOTYPES
void *initAudio(void *ptr);
void *initGUI(void *ptr);
void *initNEAT(void *ptr);

// NEAT METHOD PROTOTYPES
Population *evolveMusicVisualizer(int gens);
bool evaluateMusicVisualizer(Organism *org);
int musicVisualizerEpoch(Population *pop,int generation,char *filename,int &winnernum,int &winnergenes,int &winnernodes);

// AUDIO METHOD PROTOTYPES
int playAudio(const char *filePath);
int playFrames();

// GUI METHOD PROTOTYPES

// SHARED DATA
static float* frames = NULL;
static VisApp* gui;

struct args {
	int argc;
	char **argv;
};

int main(int argc, char *argv[]) {
	// initialize threads
	pthread_t audioThread, guiThread, neatThread;
	int audRet, guiRet, neatRet;
    pthread_mutex_t newestImageLock;
    pthread_cond_t imageAvailable;
	
    pthread_mutex_init(&newestImageLock, NULL);

    args init;
	init.argc = argc;
	init.argv = argv;

    gui = new VisApp();

	// create threads
	audRet = pthread_create( &audioThread, NULL, initAudio, (void *) &init);
	guiRet = pthread_create( &guiThread, NULL, initGUI, (void *) &init);
    neatRet = pthread_create( &neatThread, NULL, initNEAT, (void *) &init);

	// wait for threads to be done
	pthread_join( audioThread, NULL );
	pthread_join( guiThread, NULL );
    pthread_join( neatThread, NULL );

    pthread_mutex_destroy(&newestImageLock);

	// return result
	return (audRet && guiRet && neatRet);
}

/*
    THREAD INIT METHODS
*/
void *initNEAT(void *ptr) {
    args *init;
	init = (args *) arguments;

    NEAT::Population *p = 0;
    NEAT::load_neat_params(argv[2], true);
    p = evolveMusicVisualizer(100);

    if (p)
        delete p;
}

void *initAudio(void *ptr) {
    args *init;
	init = (args *) arguments;

	playAudio(argv[1]);
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
}

/*
    NEAT Methods
*/
Population *evolveMusicVisualizer(int gens) {
    Population *pop=0;
    Genome *start_genome;
    char curword[20];
    int id;

    ostringstream *fnamebuf;
    int gen;
 
    int evals[NEAT::num_runs];  //Hold records for each run
    int genes[NEAT::num_runs];
    int nodes[NEAT::num_runs];
    int winnernum;
    int winnergenes;
    int winnernodes;
    //For averaging
    int totalevals=0;
    int totalgenes=0;
    int totalnodes=0;
    int expcount;
    int samples;  //For averaging

    memset (evals, 0, NEAT::num_runs * sizeof(int));
    memset (genes, 0, NEAT::num_runs * sizeof(int));
    memset (nodes, 0, NEAT::num_runs * sizeof(int));

    ifstream iFile("musicvisualizerstartgenes",ios::in);

    cout<<"START VISUALIZATION TEST"<<endl;

    cout<<"Reading in the start genome"<<endl;
    //Read in the start Genome
    iFile>>curword;
    iFile>>id;
    cout<<"Reading in Genome id "<<id<<endl;
    start_genome=new Genome(id,iFile);
    iFile.close();

    for(expcount=0;expcount<NEAT::num_runs;expcount++) {
      //Spawn the Population
      cout<<"Spawning Population off Genome2"<<endl;

      pop=new Population(start_genome,NEAT::pop_size);
      
      cout<<"Verifying Spawned Pop"<<endl;
      pop->verify();
      
      for (gen=1;gen<=gens;gen++) {
	cout<<"Epoch "<<gen<<endl;	

	//This is how to make a custom filename
	fnamebuf=new ostringstream();
	(*fnamebuf)<<"gen_"<<gen<<ends;  //needs end marker

	#ifndef NO_SCREEN_OUT
	cout<<"name of fname: "<<fnamebuf->str()<<endl;
	#endif

	char temp[50];
	sprintf (temp, "gen_%d", gen);

	//Check for success
	if (musicVisualizerEpoch(pop,gen,temp,winnernum,winnergenes,winnernodes)) {
	  //Collect Stats on end of experiment
	  evals[expcount]=NEAT::pop_size*(gen-1)+winnernum;
	  genes[expcount]=winnergenes;
	  nodes[expcount]=winnernodes;
	  gen=gens;
	}
	
	//Clear output filename
	fnamebuf->clear();
	delete fnamebuf;
	
      }

      if (expcount<NEAT::num_runs-1) delete pop;
      
    }

    //Average and print stats
    cout<<"Nodes: "<<endl;
    for(expcount=0;expcount<NEAT::num_runs;expcount++) {
      cout<<nodes[expcount]<<endl;
      totalnodes+=nodes[expcount];
    }
    
    cout<<"Genes: "<<endl;
    for(expcount=0;expcount<NEAT::num_runs;expcount++) {
      cout<<genes[expcount]<<endl;
      totalgenes+=genes[expcount];
    }
    
    cout<<"Evals "<<endl;
    samples=0;
    for(expcount=0;expcount<NEAT::num_runs;expcount++) {
      cout<<evals[expcount]<<endl;
      if (evals[expcount]>0)
      {
	totalevals+=evals[expcount];
	samples++;
      }
    }

    cout<<"Failures: "<<(NEAT::num_runs-samples)<<" out of "<<NEAT::num_runs<<" runs"<<endl;
    cout<<"Average Nodes: "<<(samples>0 ? (double)totalnodes/samples : 0)<<endl;
    cout<<"Average Genes: "<<(samples>0 ? (double)totalgenes/samples : 0)<<endl;
    cout<<"Average Evals: "<<(samples>0 ? (double)totalevals/samples : 0)<<endl;

    return pop;
}

bool evaluateMusicVisualizer(Organism *org) {
  Network *net;
  double out[100]; //The four outputs
  double this_out; //The current output
  double errorsum;

  bool success;  //Check for successful activation
  int numnodes;  /* Used to figure out how many nodes
		    should be visited during activation */

  int net_depth; //The max depth of the network to be activated
  int relax; //Activates until relaxation

  double signal = **framePtr;

  net=org->net;
  numnodes=((org->gnome)->nodes).size();

  net_depth=net->max_depth();

  wxImage *image = new wxImage(1, 100, true);
  //Load and activate the network on each input
  for(unsigned y = 0; y < 100; y++) {
    double in[2];
    in[0] = signal;
    in[1] = (double) y;

    net->load_sensors(in);

    //Relax net and get output
    success=net->activate();

    //use depth to ensure relaxation
    for (relax=0;relax<=net_depth;relax++) {
      success=net->activate();
      this_out=(*(net->outputs.begin()))->activation;
    }

    out[y]=(*(net->outputs.begin()))->activation;

    net->flush();
  }

  for(unsigned y = 0; y < 100; y++) {
    image->SetRGB(0, y, (unsigned char) (out[y][0] * 256), (unsigned char) (out[y][1] * 256), (unsigned char) (out[y][2] * 256));
  }
  return false;
/*  
  if (success) {
    errorsum=(fabs(out[0])+fabs(1.0-out[1])+fabs(1.0-out[2])+fabs(out[3]));
    org->fitness=pow((4.0-errorsum),2);
    org->error=errorsum;
  }
  else {
    //The network is flawed (shouldnt happen)
    errorsum=999.0;
    org->fitness=0.001;
  }

  #ifndef NO_SCREEN_OUT
  cout<<"Org "<<(org->gnome)->genome_id<<"                                     error: "<<errorsum<<"  ["<<out[0]<<" "<<out[1]<<" "<<out[2]<<" "<<out[3]<<"]"<<endl;
  cout<<"Org "<<(org->gnome)->genome_id<<"                                     fitness: "<<org->fitness<<endl;
  #endif

  //  if (errorsum<0.05) { 
  //if (errorsum<0.2) {
  if ((out[0]<0.5)&&(out[1]>=0.5)&&(out[2]>=0.5)&&(out[3]<0.5)) {
    org->winner=true;
    return true;
  }
  else {
    org->winner=false;
    return false;
  }
  */
}

int musicVisualizerEpoch(Population *pop,int generation,char *filename,int &winnernum,int &winnergenes,int &winnernodes) {
  vector<Organism*>::iterator curorg;
  vector<Species*>::iterator curspecies;
  //char cfilename[100];
  //strncpy( cfilename, filename.c_str(), 100 );

  //ofstream cfilename(filename.c_str());

  bool win=false;


  //Evaluate each organism on a test
  for(curorg=(pop->organisms).begin();curorg!=(pop->organisms).end();++curorg) {
    if (xor_evaluate(*curorg)) {
      win=true;
      winnernum=(*curorg)->gnome->genome_id;
      winnergenes=(*curorg)->gnome->extrons();
      winnernodes=((*curorg)->gnome->nodes).size();
      if (winnernodes==5) {
	//You could dump out optimal genomes here if desired
	//(*curorg)->gnome->print_to_filename("xor_optimal");
	//cout<<"DUMPED OPTIMAL"<<endl;
      }
    }
  }
  
  //Average and max their fitnesses for dumping to file and snapshot
  for(curspecies=(pop->species).begin();curspecies!=(pop->species).end();++curspecies) {

    //This experiment control routine issues commands to collect ave
    //and max fitness, as opposed to having the snapshot do it, 
    //because this allows flexibility in terms of what time
    //to observe fitnesses at

    (*curspecies)->compute_average_fitness();
    (*curspecies)->compute_max_fitness();
  }

  //Take a snapshot of the population, so that it can be
  //visualized later on
  //if ((generation%1)==0)
  //  pop->snapshot();

  //Only print to file every print_every generations
  if  (win||
       ((generation%(NEAT::print_every))==0))
    pop->print_to_file_by_species(filename);


  if (win) {
    for(curorg=(pop->organisms).begin();curorg!=(pop->organisms).end();++curorg) {
      if ((*curorg)->winner) {
	cout<<"WINNER IS #"<<((*curorg)->gnome)->genome_id<<endl;
	//Prints the winner to file
	//IMPORTANT: This causes generational file output!
	print_Genome_tofile((*curorg)->gnome,"xor_winner");
      }
    }
    
  }

  pop->epoch(generation);

  if (win) return 1;
  else return 0;

}

/*
    AUDIO METHODS
*/
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
