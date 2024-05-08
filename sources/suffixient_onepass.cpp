// Copyright (c) 2024, Nicola Prezza.  All rights reserved.
// Use of this source code is governed
// by a MIT license that can be found in the LICENSE file.

#include <iostream>
#include <sdsl/construct.hpp>
#include <set>
#include <limits>
#include <algorithm>

using namespace std;
using namespace sdsl;

int_vector<8> T;
int_vector_buffer<> SA;
int_vector_buffer<> LCP;

uint8_t bwt(uint64_t i){
	return SA[i]==0 ? 0 : T[SA[i]-1];
}

struct lcp_maxima{
	int64_t lcp;
	uint64_t text_pos;
	bool saved;
};

void help(){

	cout << "suffixient [options]" << endl <<
	"Input: non-empty ASCII file without character 0x0, from standard input. Output: smallest suffixient-nexessary set." << endl <<
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

		append_zero_symbol(T);
		store_to_cache(T, conf::KEY_TEXT, cc);
		construct_sa<8>(cc);
		construct_lcp_kasai<8>(cc);
		SA = int_vector_buffer<>(cache_file_name(conf::KEY_SA, cc));
		LCP = int_vector_buffer<>(cache_file_name(conf::KEY_LCP, cc));
	}

	vector<lcp_maxima> r_ext(sigma,{-1,0,true}); //vector with candidate suffixient right-extensions
	vector<uint64_t> suffixient;

	/*
	* Algorithm: for every character C, focus on the BWT(rev(T)) C-run borders, i.e. positions
	* i such that BWT[i-1,i] = xC or BWT[i-1,i] = Cx (x \neq C). Let LCP[l,...,r] be the maximal
	* substring of LCP with l <= i <= r such that LCP[l,...,r] >= LCP[i]. If BWT[l,...,r] does not 
	* contain another C-run border BWT[j-1,j] such that LCP[j]>LCP[i], then the text position
	* corresponding to i or i-1 (the one containing C in the BWT) is in the suffixient set.
	*
	* Why does it work? w.l.o.g., assume BWT[i-1,i]=xC. If BWT position i is inserted in the
	* suffixient set, this testifies a right-maximal string X of length LCP[i] having right extension 
	* C (i.e. X.C appears in the text). Moreover, the above conditions guarantee that no other 
	* extension Y.C of another right-maximal string Y is such that X.C suffixes Y.C. This can be
	* proved to give the smallest suffixient set.
	*/

	//main algorithm: compute suffixient-nexessary set by scanning SA, LCP, and BWT
	uint64_t i=1;
	
	while(i<N){

		int64_t min_lcp = N;

		//skip over BWT equal-letter runs; just record the minimum LCP inside the run
		while(i < N and bwt(i) == bwt(i-1)){
			min_lcp = min(min_lcp,int64_t(LCP[i]));
			i++;
		}

		//here we are either at a run border or past the end of the BWT

		if(min_lcp<N)//if we just scanned a BWT run of length > 1
			for(uint8_t c = 1; c<sigma;++c)
				if(min_lcp < r_ext[c].lcp){

					if(not r_ext[c].saved)
						suffixient.push_back(r_ext[c].text_pos);

					r_ext[c] = {min_lcp,0,true};

				}

		//BWT[i-1,i] is a BWT run
		if(i<N){

			bwtruns++;

			if(int64_t(LCP[i]) > r_ext[bwt(i-1)].lcp) 
				r_ext[bwt(i-1)] = {int64_t(LCP[i]),N - SA[i-1] - 1,false};
			
			if(int64_t(LCP[i]) > r_ext[bwt(i)].lcp) 
				r_ext[bwt(i)] = {int64_t(LCP[i]),N - SA[i] - 1,false};		

			for(uint8_t c = 1; c<sigma;++c)
				if(int64_t(LCP[i]) < r_ext[c].lcp){

					if(not r_ext[c].saved)
						suffixient.push_back(r_ext[c].text_pos);

					r_ext[c] = {int64_t(LCP[i]),0,true};

				}

		}
		
		i++;

	}

	//save residuals right-extensions
	for(uint8_t c = 1; c<sigma;++c){

		if(not r_ext[c].saved)
			suffixient.push_back(r_ext[c].text_pos);

	}	

	sdsl::remove(cache_file_name(conf::KEY_TEXT, cc));
	sdsl::remove(cache_file_name(conf::KEY_SA, cc));
	sdsl::remove(cache_file_name(conf::KEY_ISA, cc));
	sdsl::remove(cache_file_name(conf::KEY_LCP, cc));

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