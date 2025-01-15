
/* DYNAMIC END-TAGGED DENSE CODE. -- 
A dynamic word-based byte oriented compressor for text files based on 
dynamic End-Tagged Dense Code.

Brisaboa, N. R., Fari�a, A., Navarro, G., Param�, J. R. 
Simple, Fast, and Efficient Natural Language Adaptive Compression. 
11th International Symposium on String Processing and Information Retrieval (SPIRE'04) - LNCS 3246. A. Apostolico, M. Melucci (Ed.), pp. 230-241. 
Padova (Italia), 2004. 

Copyright (C) 2005 Antonio Fari�a.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Author's contact: Antonio Fari�a, Dept. of Computer Science, University of
A Coru�a. Campus de Elvi�a s/n. Spain  fari@udc.es
*/


#ifndef HASHWM
#define HASHWM

#include <string.h>
#include <stdlib.h>
#include <math.h>
//#include <malloc.h>
#include <cstdlib>

#include "MemoryManager.h"

#define JUMP 101  		 //jump done when a collision appears
#define OCUP_HASH 1.5	 	 //index of occupation of the hash table
#define SMALL_PRIME 1009 // a small prime number, used to compute a hash function
#define SEED	1159241
/* Type definitions */

#define MIN(a,b) (a < b) ? a : b

struct hashNode {
	  unsigned char *word;
	  unsigned long len;
	  unsigned long posInVoc;	  //positon of the canonical word in vector posInHT
};
typedef struct hashNode t_hashNode;

struct hashStr {
	t_hashNode *hash;		  /* the slots in the hash table 	*/
	unsigned long SIZE_HASH;   /* # entries in the hash table    */
	unsigned long NumElem;    /* # elements already added to the hash table*/
	MemoryManager _memMgr; 	  /* Holds dynamic memory allocation for words. */	
};

typedef struct hashStr *t_hash;

class HashWM
{
public:
	static t_hash initialize_hash (unsigned long sizeVoc);
	static void insertElement (t_hash h, const unsigned char *aWord, register unsigned long len, register unsigned long *addr);
	static unsigned long search (t_hash h, const unsigned char *aWord, register unsigned len, unsigned long *returnedAddr);
	static unsigned long inHashTable (t_hash h, const unsigned char *aWord, register unsigned len, unsigned long *returnedAddr);
	static void destroy_hash (t_hash hash);
	static int strcomp(const unsigned char *s1, const unsigned char *s2, register unsigned long ws1, unsigned long ws2);
	static int strcompL(const uchar *s1, const uchar *s2, register unsigned long ws1, register unsigned long ws2);

private:
	static unsigned long NearestPrime (unsigned long n);
	static unsigned long hashFunction (const unsigned char *aWord, unsigned int len, unsigned long sizeHash);
};
#endif
