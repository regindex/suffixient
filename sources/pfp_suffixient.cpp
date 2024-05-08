#include<iostream>

//#define VERBOSE

#include <common.hpp>

#include <sdsl/rmq_support.hpp>
#include <sdsl/int_vector.hpp>
#include <sdsl/io.hpp>

#include <pfp.hpp>
#include <pfp_iterator.hpp>

#include <malloc_count.h>

struct lcp_maxima{
  int64_t lcp;
  uint64_t text_pos;
  bool saved;
};

void help(){

  std::cout << "suffixient [options]" << std::endl <<
  "Input: Path to PFP data structures. Output: smallest suffixient-nexessary set." << std::endl <<
  "Options:" << std::endl <<
  "-h          Print usage info." << std::endl << 
  "-i <arg>    Basepath for the PFP data structures." << std::endl << 
  "-o <arg>    Store output to file using 64-bits unsigned integers. If not specified, output is streamed to standard output in human-readable format." << std::endl <<
  "-w <arg>    Trigger string size." << std::endl << 
  "-n <arg>    Text length." << std::endl <<
  "-s          Sort output. Default: false." << std::endl <<
  "-p          Print to standard output size of suffixient set. Default: false." << std::endl <<
  "-r          Print to standard output number of equal-letter runs in the BWT of reverse text. Default: false." << std::endl;
  exit(0);
}

int main(int argc, char* const argv[])
{
  if(argc<2) help();

  std::string output_file, input_path;

  bool sort=false;
  bool rho=false;
  bool runs=false;

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

  // compute PFP data structures
  pf_parsing pf(input_path, w);

  // compute PFP iterator
  pfp_iterator iter(pf, input_path);

  /*
  * algorithm: compute suffixient-nexessary set by streaming SA, LCP, and BWT using the PFP data structures.
  */

  // first index
  uint64_t i=1;
  char p = iter.get_bwt(), c;
  uint64_t p_sa = iter.get_sa(), c_sa;
  ++iter; // move forward pfp iterator by one position
  // alphabet size
  int sigma = 128; 
  uint64_t bwtruns=0;

  //vector with candidate suffixient right-extensions
  std::vector<lcp_maxima> r_ext(sigma,{-1,0,true});
  std::vector<uint64_t> suffixient;
  
  // iterata until all values have been streamed
  while(not iter.is_finished()){

    int64_t min_lcp = -1;
    c = iter.get_bwt();
    c_sa = iter.get_sa();
    // flag marking if we have visited a run with length > 1
    if(p == c)
    {
        min_lcp = iter.get_lcp();
        ++iter; p = c; p_sa = c_sa;
        while(not iter.is_finished() and p == iter.get_bwt())
        {
            c = iter.get_bwt(); c_sa = iter.get_sa();
            min_lcp = std::min(min_lcp,int64_t(iter.get_lcp()));
            ++iter; p = c; p_sa = c_sa;
        }
        c = iter.get_bwt();
        c_sa = iter.get_sa();
    }

    //here we are either at a run border or past the end of the BWT

    if(min_lcp > -1) //if we just scanned a BWT run of length > 1
      for(uint8_t c = 1; c < sigma; ++c)
        if(min_lcp < r_ext[c].lcp){

          if(not r_ext[c].saved)
            suffixient.push_back(r_ext[c].text_pos);

          r_ext[c] = {min_lcp,0,true};

        }

    //BWT[i-1,i] is a BWT run
    if(not iter.is_finished()){

      bwtruns++;

      if(int64_t(iter.get_lcp()) > r_ext[p].lcp) 
        r_ext[p] = {int64_t(iter.get_lcp()),N - p_sa - 1,false};
      
      if(int64_t(iter.get_lcp()) > r_ext[c].lcp) 
        r_ext[c] = {int64_t(iter.get_lcp()),N - c_sa - 1,false};    

      for(uint8_t c = 1; c < sigma; ++c)
        if(int64_t(iter.get_lcp()) < r_ext[c].lcp){

          if(not r_ext[c].saved)
            suffixient.push_back(r_ext[c].text_pos);

          r_ext[c] = {int64_t(iter.get_lcp()),0,true};

        }

    }
    
    ++iter; p = c; p_sa = c_sa;

  }

  //save residuals right-extensions
  for(uint8_t c = 1; c < sigma; ++c){

    if(not r_ext[c].saved)
      suffixient.push_back(r_ext[c].text_pos);

  } 

  if(sort) std::sort(suffixient.begin(),suffixient.end());

  std::cout << "\nSmallest suffixient set: ";
  if(output_file.length()==0){

      for(auto x:suffixient) std::cout << x << " ";
      std::cout << std::endl;

  }else{

      uint64_t size = suffixient.size();
      std::ofstream ofs(output_file, std::ios::binary);
      ofs.write((char*)&size, sizeof(size));
      ofs.write((char*)suffixient.data(), sizeof(uint64_t)*size);
    
  }

  std::cout << "Size of smallest suffixient set: " << suffixient.size() << std::endl;
  std::cout << "Number of equal-letter BWT(rev(T)) runs: " << bwtruns << std::endl;
  
  return 0;
}