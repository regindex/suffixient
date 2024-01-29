// Copyright (c) 2024, Nicola Prezza.  All rights reserved.
// Use of this source code is governed
// by a MIT license that can be found in the LICENSE file.

#include <iostream>
#include <sdsl/construct.hpp>
#include <set>
#include <limits>

using namespace std;
using namespace sdsl;

static constexpr uint64_t null = std::numeric_limits<uint64_t>::max();

struct lcp_maxima{
	uint64_t lcp;
	uint8_t bwt_c;
};

void help(){

	cout << "suffixient [options]" << endl <<
	"Input: non-empty ASCII file without character 0x0, from standard input. Smallest suffixient-nexessary set." << endl <<
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
	int_vector_buffer<> LCP;
	cache_config cc;
	uint64_t N = 0; //including 0x0 terminator
	uint8_t sigma = 1; // alphabet size (including terminator 0x0)

	{
		string in;
		getline(cin,in,char(0));
		N = in.size()+1;

		if(N<2){
			cerr << "Error: empty text" <<  endl;
			help();
		}

		T = int_vector<8>(N-1);

		vector<uint8_t> char_to_int(256,0); //map chars to 0...sigma-1. 0 is reserved for term.
		
		for(uint64_t i=0;i<N-1;++i){

			uint8_t c = in[N-i-2];
			if(char_to_int[c]==0) char_to_int[c] = sigma++;
			T[i] = char_to_int[c];

		}

		//cout << "sigma = " << int(sigma) << endl;

		append_zero_symbol(T);
		store_to_cache(T, conf::KEY_TEXT, cc);
		construct_sa<8>(cc);
		construct_lcp_kasai<8>(cc);
		SA = int_vector_buffer<>(cache_file_name(conf::KEY_SA, cc));
		LCP = int_vector_buffer<>(cache_file_name(conf::KEY_LCP, cc));
	}
		
	cout << "T = ";
	for(int i=0;i<N;++i) cout << (T[i]==0 ? int(0) : int(T[i]));
	cout<< endl << "SA = ";
	for(int i=0;i<N;++i) cout << SA[i] << " ";
	cout<< endl << "LCP = ";
	for(int i=0;i<N;++i) cout << LCP[i] << " ";	
	cout << endl;

	uint8_t bwt_prev = T[N-2]; //previous BWT character
	uint64_t t_pos_prev = 0; // position in the original text (before reversing) corresponding to bwt_prev

	vector<lcp_maxima>(sigma,{null,0});

	int lcp_derivative = 0; //is the lcp stalling (0), increasing (1) or decreasing (-1)?

	//scan SA, LCP, BWT
	for(uint64_t i=1;i<N;++i){

		uint8_t bwt_curr = SA[i]==0 ? 0 : T[SA[i]-1]; //current BWT character
		uint64_t t_pos_curr = N - SA[i] - 1;   // position in the original text (before reversing) corresponding to bwt_curr



		bwt_prev = bwt_curr;
		t_pos_prev = t_pos_curr;
	
	}

	//cout << endl;

	sdsl::remove(cache_file_name(conf::KEY_TEXT, cc));
	sdsl::remove(cache_file_name(conf::KEY_SA, cc));

}

