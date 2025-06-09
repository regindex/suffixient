// Copyright (c) 2025, Francisco Olivares.
// All rights reserved.
// Use of this source code is governed
// by a MIT license that can be found in the LICENSE file.

#include <iostream>
#include <sdsl/construct.hpp>
#include <set>
#include <limits>
#include <algorithm>
#include <ctime>
#include <unistd.h>

using namespace std;
using namespace sdsl;

int_vector<8> T_rev;
int_vector_buffer<> SA;
int_vector_buffer<> LCP_;

inline uint8_t BWT(uint64_t i){
  return SA[i] == 0 ? 0 : T_rev[SA[i] - 1];
}


void help(){

  cout << "suffixient [options]" << endl <<
  "Input: non-empty ASCII file without character 0x0, from standard input. Output: smallest suffixient-nexessary set." << endl <<
  "Warning: if 0x0 appears, the standard input is read only until the first occurrence of 0x0 (excluded)." << endl <<
  "Options:" << endl <<
  "-h          Print usage info." << endl << 
  "-i          input file suffixient set." << endl << 
  "-o <arg>    The file containing the set being tested." << endl;
  exit(0);
}


inline vector<uint64_t> map_to_BWT(vector<uint64_t> & S, vector<uint64_t> & ISA,\
  uint64_t N) {
  vector<uint64_t> mapped;
  for (auto element:S) {
    mapped.push_back(ISA[N - element]);
  }
  return mapped;
}

inline uint64_t argmin(vector <int64_t> & LCP, uint64_t i, uint64_t j) {
  if (LCP[i] < LCP[j]) {
    return i;
  }
  else {
    return j;
  }
}

inline uint64_t argmax(vector <int64_t> & LCP, uint64_t i, uint64_t j) {
  if (LCP[i] <= LCP[j]) {
    return j;
  }
  else {
    return i;
  }
}

inline vector<uint64_t> compute_LPR(vector <int64_t> & LCP) {
  vector <uint64_t> LPR(LCP.size(), 0);
  vector <uint64_t> APR(LCP.size(), 0);
  vector <uint64_t> BPR(LCP.size(), 0);
  for(uint64_t i = 1; i < LCP.size(); i++) {
    if(BWT(i) != BWT(i - 1)) {
      BPR[i] = i;
    }
    else {
      BPR[i] = argmin(LCP, BPR[i - 1], i);
    }
  }
  for(int64_t j = LCP.size() - 2; j >= 0; j--) {
    if(BWT(j) != BWT(j + 1)) {
      APR[j] = j + 1;
    }
    else {
      APR[j] = argmin(LCP, j + 1, APR[j + 1]);
    }
  }
  for (uint64_t i = 0; i < LCP.size(); i++) {
    LPR[i] = argmax(LCP, APR[i], BPR[i]);
  }
  return LPR;

}


vector<vector<uint64_t>> classify_by_bwt_symbol(vector<uint64_t> & S, uint64_t N,\
  vector<int64_t> & LCP, uint8_t sigma) {
  vector<vector<uint64_t>> classified;
  vector<uint64_t> LPR = compute_LPR(LCP);
  for (uint64_t i = 0; i < sigma; i++) {
    classified.push_back(vector<uint64_t> ());
  }
  for (auto element:S) {
    classified[BWT(element)].push_back(LPR[element]);
  }
  return classified;
}


bool suffixiency(vector<vector<uint64_t>> C, uint64_t N, vector<int64_t> & PSV, \
  vector<int64_t> & NSV, uint8_t sigma) {

  vector<int64_t> P(sigma);
  for (uint64_t i = 1; i < N; i++) {
    if (BWT(i - 1) != BWT(i)) {
      for (uint64_t ip = i - 1; ip <= i; ip++) {
        if (BWT(ip) != 0) {
          if (C[BWT(ip)].empty()) {
            return false;
          }
          while (int64_t(C[BWT(ip)][P[BWT(ip)]]) <= PSV[i]) {
            if (P[BWT(ip)] == C[BWT(ip)].size() - 1) {
              return false;
            }
            P[BWT(ip)]++;
          }
            if (NSV[i] <= C[BWT(ip)][P[BWT(ip)]]) {
              return false;
            }
        }
      }
    }
  }
  return true;
}

bool minimality(vector<vector<uint64_t>> C, vector<int64_t> & PSV, \
  vector<int64_t> & NSV, uint8_t sigma) {
  for(auto symbol_list:C) {
    for (uint64_t i = 1; i < symbol_list.size(); i++) {
      if (PSV[symbol_list[i]] < NSV[symbol_list[i - 1]]) {
        return false;
      }
    }
  }
  return true;
}

// modifies psv and nsv vectors with PSV(LCP) and NSV(LCP), respectively
void sv(vector<int64_t> LCP, vector<int64_t> & psv, vector<int64_t> & nsv) {
  uint64_t N = LCP.size() - 1;
  vector<uint64_t> stack_nsv;
  vector<uint64_t> stack_psv;
  nsv = vector<int64_t>(N, N);
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
  srand(time(NULL));
  cache_config cc;
  string input_file;

  int opt;
  while ((opt = getopt(argc, argv, "prshto:")) != -1){
    switch (opt){
      case 'h':
        help();
      break;
      case 'o':
        input_file = string(optarg);
      break;
      default:
        help();
      return -1;
    }
  }

  vector<uint64_t> S;
  uint64_t N = 0;
  ifstream file(input_file);
  string line;
  if (file.is_open()) {
    uint64_t size;
    file.read(reinterpret_cast<char*>(&size), sizeof(size));
    S.resize(size);
    file.read(reinterpret_cast<char*>(S.data()), sizeof(uint64_t)*size);
    file.close();
  }
  string in;

  getline(cin,in,char(0));

  N = in.size() + 1;
  uint8_t sigma = 1; 
  T_rev = int_vector<8>(N - 1);

  vector<uint8_t> char_to_int(256, 0);
  
  for(uint64_t i = 0; i < N - 1; ++i)
  {
      uint8_t c = in[N - i - 2];
      if(char_to_int[c] == 0) char_to_int[c] = sigma++;
      T_rev[i] = char_to_int[c];
  }

  append_zero_symbol(T_rev);
  store_to_cache(T_rev, conf::KEY_TEXT, cc);
  construct_sa<8>(cc);
  construct_lcp_kasai<8>(cc);
  SA = int_vector_buffer<>(cache_file_name(conf::KEY_SA, cc));
  LCP_ = int_vector_buffer<>(cache_file_name(conf::KEY_LCP, cc));

  // insert LCP array in a C++ vector
  std::vector<int64_t> LCP(N + 1,-1);
  for(uint64_t i=0;i<=N;++i)
    LCP[i] = LCP_[i];

  vector<int64_t> NSV, PSV;
  //sv modifies PSV and NSV vectors
  sv(LCP, PSV, NSV);

  // inverse suffix-array for T^{rev}
  vector <uint64_t> ISA(N);
  for(uint64_t i = 0; i < N; i++) {
    ISA[SA[i]] = i;
  }

  vector<uint64_t> A = map_to_BWT(S, ISA, N);
  std::sort(A.begin(), A.end());
  vector<vector<uint64_t>> C = classify_by_bwt_symbol(A, N, LCP, sigma);

  if (!suffixiency(C, N, PSV, NSV, sigma)) {
    cerr << "The given set is not suffixient." << endl;
    return 1;
  }
  if (!minimality(C, PSV, NSV, sigma)) {
    cerr << "The given set is suffixient but not of smallest cardinality." << endl;
    return 2;
  }

  cout << "The given set is suffixient of smallest cardinality" << endl;

  // remove chached files
  sdsl::remove(cache_file_name(conf::KEY_TEXT, cc));
  sdsl::remove(cache_file_name(conf::KEY_SA, cc));
  sdsl::remove(cache_file_name(conf::KEY_ISA, cc));
  sdsl::remove(cache_file_name(conf::KEY_LCP, cc));

  return 0;
}

