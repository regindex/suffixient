// Copyright (c) 2024, REGINDEX.  All rights reserved.
// Use of this source code is governed
// by a MIT license that can be found in the LICENSE file.

#include <iostream>
#include <limits>

#include <common.hpp>

#include <sdsl/rmq_support.hpp>
#include <sdsl/int_vector.hpp>
#include <sdsl/io.hpp>

#include <pfp.hpp>
#include <pfp_iterator.hpp>

#include <malloc_count.h>

struct lcp_maxima
{
  int64_t len;
  uint64_t pos;
  bool active;
};

constexpr int sigma = 128; 

void help(){

  std::cout << "suffixient [options]" << std::endl <<
  "Input: Path to PFP data structures. Output: smallest suffixient set." << std::endl <<
  "Options:" << std::endl <<
  "-h          Print usage info." << std::endl << 
  "-i <arg>    Basepath for the PFP data structures." << std::endl << 
  "-o <arg>    Store output to file using 64-bits unsigned integers. If not specified, output is streamed to standard output in human-readable format." << std::endl <<
  "-w <arg>    PFP trigger string size." << std::endl << 
  "-n <arg>    Text length." << std::endl <<
  "-p          Print to standard output size of suffixient set. Default: false." << std::endl <<
  "-r          Print to standard output number of equal-letter runs in the BWT of reverse text. Default: false." << std::endl;
  exit(0);
}

inline void eval(int64_t l, uint64_t& size, std::vector<lcp_maxima>& R, 
                 std::string output_file, FILE *suffixient_file)
{
  for(uint8_t c = 1; c < sigma; ++c)
    if(l < R[c].len)
    {
      // process an active candidate
      if(R[c].active)
      {
        size++;
        if(output_file.length() == 0)
          std::cout << R[c].pos << " ";
        else
          if (fwrite(&R[c].pos, SSABYTES, 1, suffixient_file) != 1)
            error("S write error 1");
      }
      // update to inactive state
      R[c] = {l,0,false};
    }
}

int main(int argc, char* const argv[])
{
  if(argc<2) help();

  std::string output_file, input_path;

  bool sort=false, chi=false, runs=false;

  FILE *suffixient_file;

  int w, N;

  int opt;
  while ((opt = getopt(argc, argv, "prsho:w:n:i:")) != -1){
    switch (opt){
      case 'h':
        help();
      break;
      case 'o':
        output_file = std::string(optarg);
      break;
      case 'i':
        input_path = std::string(optarg);
      break;
      case 'w':
        w = atoi(optarg);
      break;
      case 'n':
        N = atoi(optarg);
      break;
      case 'p':
        chi = true;
      break;
      case 'r':
        runs = true;
      break;
      default:
        help();
      return -1;
    }
  }

  // compute PFP data structures
  pf_parsing pf(input_path, w);

  // compute PFP iterator
  pfp_iterator iter(pf, input_path);

  // opening output files
  if(output_file.length() != 0)
  {
    if ((suffixient_file = fopen(output_file.c_str(), "w")) == nullptr)
        error("open() file " + output_file + " failed");
  }
  else
    std::cout << "\nSmallest suffixient set: ";

  /*
  * algorithm: compute suffixient-nexessary set by streaming SA, LCP, and BWT using the PFP data structures.
  */

  // move forward pfp iterator to first position
  uint64_t i=1; ++iter; 
  char p = iter.get_bwt(), c;
  uint64_t p_sa = iter.get_sa(), c_sa;
  
  uint64_t bwtruns=1, suffixient_size=0; //tot_size = 1;
  int64_t m = std::numeric_limits<int64_t>::max();
  // vector STORING candidate suffixient right-extensions
  std::vector<lcp_maxima> r_ext(sigma,{-1,0,false});
  
  // iterate until all values have been streamed
  while( ++iter )
  {
    // read current values from the stream
    m = std::min(m,int64_t(iter.get_lcp()));
    c = iter.get_bwt();
    c_sa = iter.get_sa();
    //tot_size++;

    if(c != p)
    {
      // evaluate sigma candidates
      eval(m,suffixient_size,r_ext,output_file,suffixient_file);
      // update p and c candidates
      if(iter.get_lcp() > r_ext[p].len) 
        r_ext[p] = {iter.get_lcp(),N - p_sa,true};
      if(iter.get_lcp() > r_ext[c].len) 
        r_ext[c] = {iter.get_lcp(),N - c_sa,true};  
      // reset LCP value
      m = std::numeric_limits<int64_t>::max();
      // increment number of runs
      bwtruns++;
    }
    
    // update the previous BWT character and SA entry
    p = c; p_sa = c_sa;
  }
  // evaluate last active candidates
  eval(-1,suffixient_size,r_ext,output_file,suffixient_file);

  if(output_file.length() == 0)
      std::cout << std::endl;
  else
      fclose(suffixient_file);

  if(chi)
    std::cout << "Size of smallest suffixient set: " << suffixient_size << std::endl;
  if(runs)
    std::cout << "Number of equal-letter runs: " << bwtruns << std::endl;
  
  return 0;
}