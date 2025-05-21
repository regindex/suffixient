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
int_vector_buffer<> LCP_;

inline uint8_t BWT(uint64_t i){
	return SA[i] == 0 ? 0 : T[SA[i] - 1];
}

struct lcp_maxima{
  int64_t sa_pos;
  uint64_t text_pos;
  bool active;
  int64_t nsv;
};

void help(){

	cout << "suffixient [options]" << endl <<
	"Input: non-empty ASCII file without character 0x0, from standard input. Output: smallest suffixient set." << endl <<
	"Warning: if 0x0 appears, the standard input is read only until the first occurrence of 0x0 (excluded)." << endl <<
	"Options:" << endl <<
	"-h          Print usage info." << endl << 
	"-o <arg>    Store output to file using 64-bits unsigned integers. If not specified, output is streamed to standard output in human-readable format." << endl <<
	"-s          Sort output. Default: false." << endl <<
	"-p          Print to standard output size of suffixient set. Default: false." << endl <<
	"-r          Print to standard output number of equal-letter runs in the BWT of reverse text. Default: false." << endl <<
	"-t          Map characters to integers in the range 1,2,...,sigma. Default: false." << endl;
	exit(0);
}

// modifies psv and nsv vectors with PSV(LCP) and NSV(LCP), respectively
void sv(vector<int64_t> LCP, vector<int64_t> & psv, vector<int64_t> & nsv) {
	uint64_t N = LCP.size();
	vector<uint64_t> stack_nsv;
	vector<uint64_t> stack_psv;
	nsv = vector<int64_t>(N, N + 1);
  psv = vector<int64_t>(N, -1);
	for (int64_t i = 0; i < N; i++) {
	  while (!stack_nsv.empty() and LCP[i] < LCP[stack_nsv.back()]) {
	    nsv[stack_nsv.back()] = i;
	    stack_nsv.pop_back();
	  }
	  stack_nsv.push_back(i);
	  while(!stack_psv.empty() and LCP[i] <= LCP[stack_psv.back()]) {
	  	stack_psv.pop_back();
	  }
	  if (!stack_psv.empty()) {
	  	psv[i] = stack_psv.back();
	  }
  	stack_psv.push_back(i);
	}
}

int main(int argc, char** argv){

	if(argc > 4) help();

	string output_file;

	bool sort = false;
	bool rho = false;
	bool runs = false;
	bool remap = false;

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
				remap=true;
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

		if(not remap)
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
		LCP_ = int_vector_buffer<>(cache_file_name(conf::KEY_LCP, cc));
	}

	// insert LCP array in a C++ vector
	std::vector<int64_t> LCP(N,-1);
	for(uint64_t i=0;i<N;++i)
		LCP[i] = LCP_[i];

  int64_t infinity = std::numeric_limits<int64_t>::max();
	vector<lcp_maxima> R(sigma,{-2, 0, false, infinity}); //vector with candidate suffixient right-extensions
	vector<uint64_t> S;
  vector<int64_t> NSV, PSV;
  //sv modifies PSV and NSV vectors
  sv(LCP, PSV, NSV);

  // FM alg
  for(uint64_t i = 1; i < N; ++i) {
    if(BWT(i) != BWT(i - 1)) {
      for(int64_t ip = i - 1; ip < (i + 1); ++ip) {
        if (BWT(ip) != 0) {
          if(R[BWT(ip)].sa_pos <= PSV[i]) {
            if (R[BWT(ip)].nsv < int64_t(i)) {
              S.push_back(R[BWT(ip)].text_pos);
            }
            R[BWT(ip)] = {int64_t(i), N - SA[ip], true, NSV[i]};
          }
        }
      }
    }
  }
  // evaluate last active candidates
  for(uint8_t c = 1; c < sigma; ++c) {
    // process an active candidate
    if(R[c].active)
      S.push_back(R[c].text_pos);
  }

  // remove chached files
  sdsl::remove(cache_file_name(conf::KEY_TEXT, cc));
  sdsl::remove(cache_file_name(conf::KEY_SA, cc));
  sdsl::remove(cache_file_name(conf::KEY_ISA, cc));
  sdsl::remove(cache_file_name(conf::KEY_LCP, cc));

  if(sort) std::sort(S.begin(),S.end());

  if(output_file.length()==0){
    for(auto x:S) cout << x << " ";
    cout << endl;
  }
  else{
    uint64_t size = S.size();
    ofstream ofs(output_file, ios::binary);
    ofs.write((char*)&size, sizeof(size));
    ofs.write((char*)S.data(), sizeof(uint64_t)*size);
  }

  if(rho) cout << "Size of smallest suffixient set: " << S.size() << endl;
  if(runs) cout << "Number of equal-letter BWT(rev(T)) runs: " << bwtruns << endl;
}
