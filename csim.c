//===============================================================
// Niranjan Reji
// 04/22/2022
// reji_n1-handin.tar
// Program to simulate an LRU cache using a cache line struct
//===============================================================
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include "cachelab.h"



/*** TYPES ***/
typedef unsigned long long int ADDR_T;

// c style structs for your cache construction
typedef struct cacheLine
{
	int validbit;  // the valid bit in cache lines indicate if data is stored in the line
	ADDR_T tag; // the tag is an address pointing to the line
	int age; // this is a timestamp we use to implement LRU
} cacheLine_t;

typedef cacheLine_t* cacheSet_t;  // this is a structure that points to all of the lines in a set
typedef cacheSet_t* cache_t;  // this is a structure that points to all of the sets in a cache

/*** Global Arguments ***/
int s = 0;      		/* number of set bits */
int S;          		/* number of Sets = 2^s */
int E = 0;      		/* number of lines per set */
int b = 0;      		/* number of block bits */
int B;          		/* number of blocks = 2^b */
int verbose = 0;        /* toggle for verbose mode */
char *tracefile = 0;    /* filename of trace input file */

int num_hits = 0;
int num_misses = 0;
int num_evictions = 0;
int LRU_counter = 0;    // keep track of time stamp of each mem access


/*** function declarations ***/

cache_t cache; // we create our cache here

// the function below allocates memory for our cache's sets and lines
void	allocateCache()
{
	cache = (cacheSet_t*)malloc(sizeof(cacheSet_t)*S); // allocates memory for the sets in the cache
	for (int i=0; i<S; i++)
	{
		cache[i] = (cacheLine_t*)malloc(sizeof(cacheLine_t)*E); // allocates memory for the lines in the cache
		for (int j=0; j<E; j++) // this loops through the lines in the cache and sets default values for their attributes since all of them are empty initially.
		{
			cache[i][j].validbit = 0;
			cache[i][j].tag = 0;
			cache[i][j].age = 0;
		}
	}
}

// this function destroys the memory the cache uses
void emptyCache()
{
	for (int i=0; i<S; i++) // loop to iterate through cache sets
	{
		free(cache[i]); // frees memory set by set
	}
	free(cache); // deletes the cache
}

//this function is called when a cache access results in a miss - it takes in the set and tag from the address that resulted in a miss
void cacheMiss(cacheSet_t cacheSet, ADDR_T tag)
{
	int iterator = INT_MAX; // this value is compared with the age of each line in the set, and replaced with the age of the line with the lowest timestamp (least recent)
	int LRULine = 0; // index that is set for the least recently used line
	num_misses++; // miss counter is iterated

	for (int i=0; i<E; i++) // this loop iterates through each line to find the least recent aged line among them
	{
		if (iterator > cacheSet[i].age)
		{
			LRULine = i;
			iterator = cacheSet[i].age;
		}
	}

	if (cacheSet[LRULine].validbit) // if the least recently used line has data in it, the data there is being evicted. So, we iterate the eviction counter
	{
		num_evictions++;
	}

	cacheSet[LRULine].validbit = 1; // these 3 lines set the line's values to default values.
	cacheSet[LRULine].tag = tag;
	cacheSet[LRULine].age = LRU_counter++;
}

// this function attempts to access information in the cache using an address passed to it
void readCache(ADDR_T addr)
{
	ADDR_T set = (addr >> b) & ((1 << s)-1); // shift address to right by block bits to eliminate them, and mask with set bit number to get only set bits
	ADDR_T tag = addr >> (s+b); // shifts address to right by set and block bits to eliminate set and block bits and only retain tag
	cacheSet_t cacheSet = cache[set]; // accesses specific set inside cache

	for (int i=0; i<E; ++i)
	{
		if ((cacheSet[i].validbit) && (cacheSet[i].tag == tag)) // checks if line accessed stores information and if the tag of the line matches our tag
		{
			cacheSet[i].age = LRU_counter++;
			num_hits++;
			return;
		}
	}

	cacheMiss(cacheSet, tag); // if the line information doesn't match or the line is empty, we get a miss
	return;
}


void    printUsage      ( char *argv[] );

//===============================================================
//===============================================================
int main(int argc, char *argv[])
{

 	int c;
	while ( (c = getopt(argc,argv,"s:E:b:t:v")) != -1 )
	{
		switch (c)
		{
		case 's':	s = atoi(optarg);
					break;
		case 'E':	E = atoi(optarg);
					break;
		case 'b':	b = atoi(optarg);
					break;
		case 't':	tracefile = optarg;
					break;
		case 'v':	verbose = 1;
					break;
		case 'h':   printUsage(argv);
					exit(0);
		};
	}
	S = 1 << s;		// number of sets
	B = 1 << b;		// number of blocks per line

	// comment this out later once you are sure you have
	// command line options working.
	//printf("S[%d,%d] E[%d] B[%d,%d] v[%d] t[%s]\n",
		//S,s,E,B,b,verbose,tracefile);

	// your code here

	allocateCache();

	FILE* tracePath = fopen(tracefile, "r");
	char traceCMD;
	ADDR_T address;
	int size;

	while (fscanf(tracePath, " %c %llx,%d", &traceCMD, &address, &size) == 3)
	{
		switch(traceCMD)
		{
			case 'S': readCache(address);
			break;
			case 'L': readCache(address);
			break;
			case 'M': readCache(address);
			readCache(address);
			break;
			default: break;
		}
	}

	fclose(tracePath);
	emptyCache();

	// change this to reflect hits/misses/evictions counts
	printSummary(num_hits,num_misses,num_evictions);
    return 0;
}



//===============================================================
//===============================================================

void printUsage (char *argv[] )
{
    printf("Usage: \n");
    printf("%s -s #setbits -E #lines -b #blockbits -t tracefilename -v -h\n",argv[0]);
    printf("where s specifies number of sets S = 2^s\n");
    printf("where E specifies number of lines per set\n");
    printf("where b specifies size of blocks B = 2^b\n");
    printf("where t specifies name of tracefile\n");
    printf("where v turns no verbose mode (default is off)\n");
    printf("where h prints this help message\n");
}
