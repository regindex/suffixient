// Copyright (c) 2024, Nicola Prezza.  All rights reserved.
// Use of this source code is governed
// by a MIT license that can be found in the LICENSE file.

#include <iostream>
#include <sdsl/construct.hpp>
#include <set>
#include <limits>
#include<algorithm>

using namespace std;
using namespace sdsl;

struct lcp_maxima{
	int64_t lcp;
	uint64_t text_pos;
	bool saved;
};

void help(){

	cout << "suffixient [options]" << endl <<
	"Input: non-empty ASCII file without character 0x0, from standard input. Smallest suffixient-nexessary set." << endl <<
	"Warning: if 0x0 appears, the standard input is read only until the first occurrence of 0x0 (excluded)." << endl <<
	"Options:" << endl <<
	"-h          Print usage info." << endl << 
	"-o <arg>    Store output to file using 64-bits unsigned integers. If not specified, output is streamed to standard output in human-readable format." << endl <<
	"-s          Sort output. Default: false." << endl <<
	"-p          Print to standard output size of suffixient set. Default: false." << endl <<
	"-r          Print to standard output number of equal-letter runs in the BWT of reverse text. Default: false." << endl;
	exit(0);
}

int main(int argc, char** argv){

	if(argc>4) help();

	string output_file;

	bool sort=false;
	bool rho=false;
	bool runs=false;

	int opt;
	while ((opt = getopt(argc, argv, "prsho:")) != -1){
		switch (opt){
			case 'h':
				help();
			break;
			case 'o':
				output_file = string(optarg);
			break;
			case 's':
				sort=true;
			break;
			case 'p':
				rho=true;
			break;
			case 'r':
				runs=true;
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

	uint64_t bwtruns=0;

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
		
	/*
	cout << "T = ";
	for(int i=0;i<N;++i) cout << (T[i]==0 ? int(0) : int(T[i]));
	cout<< endl << "SA = ";
	for(int i=0;i<N;++i) cout << SA[i] << " ";
	cout<< endl << "LCP = ";
	for(int i=0;i<N;++i) cout << LCP[i] << " ";	
	cout << endl;
	*/

	uint8_t bwt_prev = T[N-2]; //previous BWT character
	uint64_t t_pos_prev = 0; // position in the original text (before reversing) corresponding to bwt_prev

	vector<lcp_maxima> r_ext(sigma,{-1,0,true}); //vector with candidate suffixient right-extensions
	vector<uint64_t> suffixient;

	//main algorithm: compute suffixient-nexessary set by scanning SA, LCP, and BWT
	for(uint64_t i=1;i<N;++i){

		uint8_t bwt_curr = SA[i]==0 ? 0 : T[SA[i]-1]; //current BWT character
		uint64_t t_pos_curr = N - SA[i] - 1;   // position in the original text (before reversing) corresponding to bwt_curr

		if(bwt_curr != bwt_prev){//BWT run break: found right-maximal string of length LCP[i]

			bwtruns++;

			if(int64_t(LCP[i]) > r_ext[bwt_prev].lcp) r_ext[bwt_prev] = {int64_t(LCP[i]),t_pos_prev,false};
			if(int64_t(LCP[i]) > r_ext[bwt_curr].lcp) r_ext[bwt_curr] = {int64_t(LCP[i]),t_pos_curr,false};		

		}

		for(uint8_t c = 0; c<sigma;++c){

			if(int64_t(LCP[i]) < r_ext[c].lcp){

				//do not save the position of the terminator
				if(not r_ext[c].saved and r_ext[c].text_pos != N-1)
					suffixient.push_back(r_ext[c].text_pos);

				r_ext[c] = {int64_t(LCP[i]),0,true};

			}

		}
		
		bwt_prev = bwt_curr;
		t_pos_prev = t_pos_curr;
	
	}


	//save residuals right-extensions
	for(uint8_t c = 0; c<sigma;++c){

		if(not r_ext[c].saved)
			suffixient.push_back(r_ext[c].text_pos);

	}	

	sdsl::remove(cache_file_name(conf::KEY_TEXT, cc));
	sdsl::remove(cache_file_name(conf::KEY_SA, cc));

	if(sort) std::sort(suffixient.begin(),suffixient.end());

	if(output_file.length()==0){

		for(auto x:suffixient) cout << x << " ";
		cout << endl;

	}else{

		uint64_t size = suffixient.size();
		ofstream ofs(output_file, ios::binary);
		ofs.write((char*)&size, sizeof(size));
		ofs.write((char*)suffixient.data(), sizeof(uint64_t)*size);
		
	}

	if(rho) cout << "Size of smallest suffixient set: " << suffixient.size() << endl;
	if(runs) cout << "Number of equal-letter BWT(rev(T)) runs: " << bwtruns << endl;

}

