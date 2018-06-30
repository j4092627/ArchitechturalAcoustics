#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>	/* true, false */
#include <string.h>	
#include "room_acoustics.h"
#include <sndfile.h>

int main(int argc, char *argv[])
{
	char *ipf,*ifilename,*ofilename;
	int i,j,count;
	Pt source,image[NUM_IMAGE];
	Path path[NUM_PATH];
	Wall wall[NUM_WALL];
	int fs,max_delay;//wav file sampling frequency
	float *ibuf;
	float *obuf;

	
	/* libsndfile data structures */
	SNDFILE *isfile,  *osfile; 
	SF_INFO isfinfo,  osfinfo;

	/* zero libsndfile structures */
	memset(&isfinfo, 0, sizeof(isfinfo));
  	memset(&osfinfo, 0, sizeof(osfinfo));

  	/*Command line checking*/
	if(argc!=4)
	{
		printf("Enter command line as ./room_acoustics parameters.txt ifile.wav ofile.wav\n");
	}
	/*parsing command line*/
	ipf = argv[1];
	ifilename = argv[2];
	ofilename = argv[3];

	/*open input wav file*/
	parse_param_file(ipf,&source,wall);
	isfile = sf_open(ifilename, SFM_READ, &isfinfo);
	if(isfile == NULL) {
		fprintf(stderr, "Error reading source file '%s': %s\n", ifilename, sf_strerror(tmpfile));//???
		return 1;
	}

	/*output file setting*/
	osfinfo.samplerate = isfinfo.samplerate;
	osfinfo.channels = isfinfo.channels;
	osfinfo.format = isfinfo.format;
	int oframes;

	/*open output file */
	osfile = sf_open(ofilename, SFM_WRITE, &osfinfo);
	fs = isfinfo.samplerate;
	/*Return max_delay*/
	max_delay = calculate_paths(&source,wall,path,image, fs);

	oframes = isfinfo.frames+max_delay;

	/* Print input file information */
	printf("Input audio file %s:\n\tFrames: %d Channels: %d Samplerate: %d\n", 
		ifilename, (int)isfinfo.frames, isfinfo.channels, isfinfo.samplerate);

	/*Allocate buffer*/
	ibuf = (float*)malloc( (max_delay+ BLOCK_LEN) * sizeof(float));//
	
	/* output is length of input + length of reverb -1 */
	obuf = (float*)malloc(BLOCK_LEN  * sizeof(float));
	memset(ibuf,0, max_delay + BLOCK_LEN);
	
	printf("Allocated buffers\n");

	/* Initial */
	bool first_read = true;
	printf("count%d\n",isfinfo.frames/BLOCK_LEN);
	//memset(obuf,0,BLOCK_LEN);

	for(count = 0; count < isfinfo.frames/BLOCK_LEN ;count++)
	{
		// if(first_read)
		// {
		// 	first_read = false;
		// 	memset(ibuf[0],0, max_delay);
		// 	sf_read_float(isfile, ibuf[0], max_delay);
		// 	printf("first_read\n");
		// }
		
		sf_read_float(isfile, &ibuf[max_delay],  BLOCK_LEN);
		
		
		for(i=0;i<BLOCK_LEN;i++)
		{
			obuf[i] = 0;
			for(j=0;j<NUM_PATH;j++)
			{
				obuf[i]+= ibuf[i+max_delay-path[j].delay]*path[j].atten;
			}
		}
		sf_write_float(osfile, obuf, BLOCK_LEN);
		for(i=0;i<max_delay;i++)
		{
			ibuf[i] = ibuf[i+BLOCK_LEN];
		}
		
	}
	
	
	
	

	sf_close (isfile);
	sf_close (osfile);
}

