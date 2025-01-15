// Copyright (c) 2024, REGINDEX.  All rights reserved.
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

#define STORE_SIZE 5

inline uint8_t BWT(uint64_t i){
	return SA[i] == 0 ? 0 : T[SA[i] - 1];
}

struct lcp_maxima{
	int64_t len;
	uint64_t pos;
	int64_t lcs;
	bool active;
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
	"-r          Print to standard output number of equal-letter runs in the BWT of reverse text. Default: false." << endl <<
	"-t          Use alphabet of size sigma (debug only). Default: false." << endl;
	exit(0);
} 

vector<uint64_t> last(128,0);
vector<uint64_t> average(128,0);
vector<uint64_t> inserted(128,0);

inline void eval(uint64_t sigma, int64_t m, vector<lcp_maxima>& R, 
								 					vector<uint64_t>& S, vector<int64_t>& L)
{
	for(uint8_t c = 1; c < sigma; ++c)
	  if(m < R[c].len)
		  {
		    // process an active candidate
		    if(R[c].active)
		    {
		    	S.push_back(R[c].pos);
		    	average[c] += R[c].lcs; inserted[c] += 1;
		    	//if(char(c) == 'A')
		    		//std::cout << R[c].lcs << " ";
		    	//
		    	if(R[c].lcs > L[c] and m != -1){ L[c] = R[c].lcs; if(char(c) == 'A' and L[c] == 33333){ std::cout << last[c] << " " << R[c].pos << std::endl; } }
		    	last[c] = R[c].pos;
		    }
		  	// evaluate LCS values
		  	// if(R[c].len > L[c]){ L[c] = R[c].len; }
		    // update to inactive state
		    R[c] = {m,0,m,false};
		  }
}

int main(int argc, char** argv){

	if(argc > 4) help();

	string output_file;

	bool sort = false;
	bool rho = false;
	bool runs = false;
	bool debug = false;

	int opt;
	while ((opt = getopt(argc, argv, "prshto:")) != -1){
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
			case 't':
				debug=true;
			break;
			default:
				help();
			return -1;
		}
	}

	cache_config cc;
	uint64_t N = 0; //including 0x0 terminator
	uint8_t sigma = 1; // alphabet size (including terminator 0x0)
	int64_t m = std::numeric_limits<int64_t>::max();
	uint64_t bwtruns = 1;

	{
		string in;
		getline(cin,in,char(0));
		N = in.size() + 1;

		if(N<2){
			cerr << "Error: empty text" <<  endl;
			help();
		}

		T = int_vector<8>(N - 1);

		vector<uint8_t> char_to_int(256, 0); //map chars to 0...sigma-1. 0 is reserved for term.

		if(debug)
		{
			for(uint64_t i = 0; i < N - 1; ++i)
			{
				uint8_t c = in[N - i - 2];
				T[i] = c;
			}
			sigma = 128;
		}
		else
			for(uint64_t i = 0; i < N - 1; ++i)
			{
				uint8_t c = in[N - i - 2];
				if(char_to_int[c] == 0) char_to_int[c] = sigma++;
				T[i] = char_to_int[c];
			}

		append_zero_symbol(T);
		store_to_cache(T, conf::KEY_TEXT, cc);
		construct_sa<8>(cc);
		construct_lcp_kasai<8>(cc);
		SA = int_vector_buffer<>(cache_file_name(conf::KEY_SA, cc));
		LCP = int_vector_buffer<>(cache_file_name(conf::KEY_LCP, cc));
	}

	vector<lcp_maxima> R(sigma,{-1,0,0,false}); //vector with candidate suffixient right-extensions
	vector<int64_t> L(sigma,-1); // vector containing the LCS values
	vector<uint64_t> S;

	for(uint64_t i=1;i<N;++i)
	{
		m = std::min(m,int64_t(LCP[i]));

		if(BWT(i) != BWT(i-1))
		{
			eval(sigma,m,R,S,L);

			for(uint64_t ip = i-1; ip < i+1; ++ip)
				if(int64_t(LCP[i]) > R[BWT(ip)].len)
					R[BWT(ip)] = {int64_t(LCP[i]),N - SA[ip],R[BWT(ip)].lcs,true}; 
      // reset LCP value
      m = std::numeric_limits<int64_t>::max();
      // increment number of runs
      bwtruns++;
		}
	}

  // evaluate last active candidates
  eval(sigma,-1,R,S,L);

  // remove chached files
  sdsl::remove(cache_file_name(conf::KEY_TEXT,cc));
  sdsl::remove(cache_file_name(conf::KEY_SA,  cc));
  sdsl::remove(cache_file_name(conf::KEY_ISA, cc));
  sdsl::remove(cache_file_name(conf::KEY_LCP, cc));

  if(sort) std::sort(S.begin(),S.end());

  if(output_file.length()==0){
    //for(auto x:S) cout << x << " ";
    //cout << endl;
  }
  else{
    ofstream ofs(output_file, ios::binary);
    for (const auto& x : S) { ofs.write(reinterpret_cast<const char*>(&x), STORE_SIZE); }
    ofs.close();
  }

  if(rho) cout << "Size of smallest suffixient set: " << S.size() << endl;
  if(runs) cout << "Number of equal-letter BWT(rev(T)) runs: " << bwtruns << endl;

  for(int i=0;i<sigma;++i)
  {
  	if(L[i] != -1)
  		std::cout << char(i) << " = " << L[i]+1 << " avg: " << double(average[i])/inserted[i] << std::endl;
  }
}