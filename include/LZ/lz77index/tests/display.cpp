#include <fstream> 
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

#include "../utils.h"
#include "../static_selfindex.h"
#include "../offsets.h"

#include <iostream>
#include <vector>
using namespace std;

/* Open patterns file and read header */
void pfile_info(unsigned int* length, unsigned int* numpatt){
	int error;
	unsigned char c;
	unsigned char origfilename[257];

	error = fscanf(stdin, "# number=%u length=%u file=%s forbidden=", numpatt, length, origfilename);
	if (error != 3)	{
		fprintf (stderr, "Error: Patterns file header not correct\n");
		perror ("run_queries");
		exit (1);
	}
	//fprintf (stderr, "# patterns = %lu length = %lu file = %s forbidden chars = ", *numpatt, *length, origfilename);
	while ( (c = fgetc(stdin)) != 0) {
		if (c == '\n') break;
		//fprintf (stderr, "%d",c);
	}
	//fprintf(stderr, "\n");
}

void pfile_info(unsigned int* numpatt){
	int error;
	unsigned char c;

	error = fscanf(stdin, "#numberOfFrases::%u", numpatt);

	if (error != 1)	{
		fprintf (stderr, "Error: Patterns file header not correct\n");
		perror ("run_queries");
		exit (1);
	}
	//fprintf (stderr, "# patterns = %lu length = %lu file = %s forbidden chars = ", *numpatt, *length, origfilename);
	while ( (c = fgetc(stdin)) != 0) {
		if (c == '\n') break;
		//fprintf (stderr, "%d",c);
	}
	//fprintf(stderr, "\n");
}

int main(int argc, char** argv)
{
	double tot_time = 0, load_time = 0, aggregated_time = 0;
    /*Load Index*/
    lz77index::utils::startTime();
	lz77index::static_selfindex* idx = lz77index::static_selfindex::load(argv[1]);
	load_time += lz77index::utils::endTime();

	int start = std::atoi(argv[2]);
	int end = std::atoi(argv[3]);

	unsigned char* result = new unsigned char(end-start+1);

	result = idx->display(start, end);

	for(size_t i=0;i<(end-start);++i)
		std::cout << *(result+i) << "";
	std::cout << std::endl;

	delete idx;
}
