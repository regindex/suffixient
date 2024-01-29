// Copyright (c) 2024, Nicola Prezza.  All rights reserved.
// Use of this source code is governed
// by a MIT license that can be found in the LICENSE file.

#include <iostream>
#include <sdsl/construct.hpp>
#include <set>

using namespace std;
using namespace sdsl;

void help(){

	cout << "suffixient [options]" << endl <<
	"Input: ASCII file without character 0x0, from standard input. Smallest suffixient-nexessary set." << endl <<
	"Warning: if 0x0 appears, the standard input is read only until the first occurrence of 0x0 (excluded)." << endl <<
	"Options:" << endl <<
	"-h          Print usage info." << endl << 
	"-o <arg>    Store output to file using 64-bits unsigned integers. If not specified, output is streamed to standard output in human-readable format." << endl;
	exit(0);
}

int main(int argc, char** argv){

	if(argc>3) help();

	string output_file;

	int opt;
	while ((opt = getopt(argc, argv, "ho:")) != -1){
		switch (opt){
			case 'h':
				help();
			break;
			case 'o':
				output_file = string(optarg);
			break;
			default:
				help();
			return -1;
		}
	}

	int_vector<8> T;
	int_vector_buffer<> SA;
	cache_config cc;
	uint64_t N = 0; //including 0x0 terminator

	{
		string in;
		getline(cin,in,char(0));
		N = in.size()+1;

		T = int_vector<8>(N-1);
		
		for(uint64_t i=0;i<N-1;++i){
			T[i] = (uint8_t)in[i];
		}

		append_zero_symbol(T);
		store_to_cache(T, conf::KEY_TEXT, cc);
		construct_sa<8>(cc);
		SA = int_vector_buffer<>(cache_file_name(conf::KEY_SA, cc));
	}
	
	/*
	for(int i=0;i<N;++i) cout << (T[i]==0 ? '#' : char(T[i]));
	cout<< endl;
	for(int i=0;i<N;++i) cout << SA[i] << endl;
	*/

	

	sdsl::remove(cache_file_name(conf::KEY_TEXT, cc));
	sdsl::remove(cache_file_name(conf::KEY_SA, cc));

}

