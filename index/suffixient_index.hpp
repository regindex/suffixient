// Copyright (c) 2024, REGZ.  All rights reserved.
// Use of this source code is governed
// by a MIT license that can be found in the LICENSE file.

/*
 *  suffixient_index: DESCRIPTION
 */

#ifndef SUFFIXIENT_INDEX_HPP_
#define SUFFIXIENT_INDEX_HPP_

#include <chrono>
#include <malloc_count.h>
#include <CTriePP.hpp>
//#include <RePairSLPIndex.h>
//#include <static_selfindex_lz77.h>
//#include <utils.h>
#include <LZ77_LCP_LCS_DS.hpp>

typedef uint32_t LUint;
typedef int32_t  Lint;
typedef int64_t  Usafe;

#define DEF_BUFFER_SIZE 1000000
#define SIGMA 128

namespace suffixient{

template<class prefixSearch, class lcpDS>
class suffixient_index{

public:
	// empty constructor
	suffixient_index(){}

	// build suffixient index by indexing the whole text prefixes
	std::pair<Usafe,Usafe> build(const std::string &text, const std::string &suffixient,
		                                                  bool verbose = false)
	{
		// BEUILD PREFIX SEARCH DATA STRUCTURE //
		std::vector<LUint> suffixient_set;
		LUint tot_inserted_char = 0, tot_inserted_keywords = 0;
	    // Open the file in binary mode
	    std::ifstream file(suffixient, std::ios::binary);
	    if (!file.is_open()) {
	        std::cerr << "Error: Could not open file " << suffixient << std::endl;
	        exit(1);
	    }

	    Usafe number;
	    size_t count = 0;

	    while (file.read(reinterpret_cast<char*>(&number), 5))
	    {
	        assert(number > 0);
	        suffixient_set.push_back(number-1);
	    }
	    if (not file.eof()) { std::cerr << "An error occurred during reading." << std::endl; }

	    file.close();

	    std::sort(suffixient_set.begin(), suffixient_set.end());

	    file = std::ifstream(text, std::ios::binary);
	    if (!file.is_open()) {
	        std::cerr << "Error: Could not open file " << text << std::endl;
	        exit(1);
	    }

	    std::string last_prefix;
	    Usafe last_index = 0;
	    for(Usafe i=0;i<suffixient_set.size();++i)
	    {
	        std::string buffer(suffixient_set[i] - last_index + 1, '0');
	        file.read(&buffer[0], suffixient_set[i] - last_index + 1);
	        last_index = suffixient_set[i]+1;
	        
	        // invert the buffer
	        std::reverse(buffer.begin(),buffer.end());
	        last_prefix = buffer + last_prefix ;

	        if(verbose)
	        	std::cout << "Insert: " << last_prefix << " : " << suffixient_set[i] << std::endl; 
	        insert_prefix(last_prefix,suffixient_set[i]);
	        tot_inserted_char += last_prefix.size();
	        tot_inserted_keywords++;
	    }

	    file.close();
	    if(verbose)
	    {
	    	print_trie();
	    	std::cout << "Tot inserted symbols: " << tot_inserted_char << std::endl;
	    	std::cout << "Tot inserted keywords: " << tot_inserted_keywords << std::endl;
	    }

	    // BEUILD LCP LCS DATA STRUCTURE //
	    G.build(text,8);

	    return std::make_pair(tot_inserted_char, tot_inserted_keywords);
	}
	
	// build suffixient index by indexing the supermaximal extensions
	std::pair<Usafe,Usafe> build(const std::string &text, const std::string &suffixient, 
								 const std::string &lcs, const std::string &first,
								 bool verbose = false)
	{
		// BEUILD PREFIX SEARCH STRUCTURE //
		std::vector<std::pair<LUint,LUint>> keywords;
		LUint tot_inserted_char = 0, tot_inserted_keywords = 0;

	    // Open the file in binary mode
	    std::ifstream file1(suffixient, std::ios::binary);
	    std::ifstream file2(lcs, std::ios::binary);
	    if (!file1.is_open() or !file2.is_open()) {
	        std::cerr << "Error: Could not open input files... " << std::endl;
	        exit(1);
	    }

	    Usafe pos, lcs_val;
	    while (file1.read(reinterpret_cast<char*>(&pos), 5) and
	    		  	  file2.read(reinterpret_cast<char*>(&lcs_val), 5))
	    {
	        assert(pos > 0 and lcs_val >= 0);

	        lcs_val += 1;
	        keywords.push_back(std::make_pair(pos-lcs_val,lcs_val));
	    }
	    if (not (file1.eof() or file2.eof()) )
	    	{ std::cerr << "An error occurred while reading the files." << std::endl; }
	    file1.close(); file2.close();

	    std::sort(keywords.begin(), keywords.end());

	    file1 = std::ifstream(text, std::ios::binary);
	    if (!file1.is_open()) {
	        std::cerr << "Error: Could not open file " << text << std::endl;
	        exit(1);
	    }

	    // compute file size
	    file1.seekg(0, std::ios::end);
	    Usafe file_size = file1.tellg();
	    file1.seekg(0, std::ios::beg);
	    // initialize text buffer
	    Usafe buffer_size = std::min(Usafe(DEF_BUFFER_SIZE),file_size);
	    Usafe curr = 0, last = buffer_size;
	    std::string buffer(buffer_size,'0');
	    file1.read(&buffer[0], buffer_size);

	    std::vector<LUint> first_keywords;
	    for(Usafe i=0;i<keywords.size();++i)
	    {
	    	Usafe start = keywords[i].first;
	    	Usafe len   = keywords[i].second;

	    	if(len > 1)
	    	{
		    	if(start+len > last)
		    	{
		    		curr = start;
		    		file1.seekg(curr, std::ios::beg);
		    		buffer_size = std::max(buffer_size,len);
		    		buffer.resize(buffer_size);
		    		file1.read(&buffer[0], buffer_size);
		    		last = curr + buffer_size;
		    	}

	    		std::string keyword(keywords[i].second, '0');
	    		for(Usafe i=0;i<keyword.size();++i)
	    			keyword[i] = buffer[start+len-i-curr-1];

		        if(verbose)
		        	std::cout << "Insert 1: " << keyword << " : " << start+len-1 << std::endl;
	        	insert_prefix(keyword,start+len-1);
	        	tot_inserted_char += keyword.size();
	        	tot_inserted_keywords++;
        	}
        	else
        		first_keywords.push_back(start);
	    }
	    file1.clear();
	    file1.seekg(0, std::ios::beg);

	    file2 = std::ifstream(first, std::ios::binary);
	    std::vector<LUint> first_lcs;
	    while (file2.read(reinterpret_cast<char*>(&lcs_val), 5))
	    	first_lcs.push_back(lcs_val+1);
	    file2.close();

	    for(Usafe i=0;i<first_keywords.size();++i)
	    {
	    	file1.seekg(first_keywords[i], std::ios::beg);
	    	char c = file1.peek();
	    	buffer.resize(first_lcs[c]);
	    	file1.seekg(first_keywords[i]-first_lcs[c]+1, std::ios::beg);
	    	file1.read(&buffer[0], first_lcs[c]);
	    	std::reverse(buffer.begin(),buffer.end());

	        if(verbose)
	        	std::cout << "Insert 2: " << buffer << " : " << first_keywords[i] << std::endl; 
	    	insert_prefix(buffer,first_keywords[i]);
        	tot_inserted_char += buffer.size();
        	tot_inserted_keywords++;
	    }

	    file1.close();
	    if(verbose)
	    {
	    	print_trie();
	    	std::cout << "Tot inserted symbols: " << tot_inserted_char << std::endl;
	    	std::cout << "Tot inserted keywords: " << tot_inserted_keywords << std::endl;
	    }

	    // BEUILD LCP LCS DATA STRUCTURE //
	    G.build(text,8);

	    //std::string pattern = "TGATGATA";
	    //std::string pattern = "TGATGATAAGGTAGGAATAG";
	    //std::string pattern = "TGATGATAATAAAGA";
	    //find_MEMs(pattern);

	    return std::make_pair(tot_inserted_char, tot_inserted_keywords);
	}	

	// function to insert a prefix - text position pair in the z-fast trie
	template<typename U>
	inline void insert_prefix(const std::string& prefix, const U& position)
	{
		Z.insert(&prefix,position);
		
		return;
	}

	// print z-fast trie
	void print_trie(){ Z.print(); }

	/* STORE AND LOAD NOT YET FINISHED
	Ulong store(std::string outputFile)
	{
		std::ofstream out(outputFile, std::ios::binary);
		Ulong size = Z.store(out);
		out.close();

		return size;
	}
	void load(std::string inputFile)
	{
		std::ifstream in(inputFile, std::ios::binary);
		Z.load(in);
		in.close();

		return ;
	} */

	// 
	Ulong locate_prefix(std::string pattern){ return Z.locatePrefix(pattern); }

	//
	std::pair<Ulong,Int> locate_longest_prefix(std::string pattern){
		return Z.locateLongestPrefix(pattern);
	}

	//
	std::string get_longest_match_seq(std::string pattern){
		return Z.getLongestMatchSeq(pattern);
	}

	void find_MEMs(string& pattern, std::ofstream& output)
	{
		////std::cout << "##### FINDING MEMs for " << pattern << std::endl;
		size_t i = 0, l = 0, m = pattern.size();
		int64_t pstart = 0;
		while(i < m)
		{
			////std::cout << "#################### " << pattern.substr(pstart,(i+1)-pstart) << std::endl;
			std::string right_max_substr = pattern.substr(pstart,(i+1)-pstart);
			std::reverse(right_max_substr.begin(),right_max_substr.end());
			//std::cout << "right maximal substring: " << right_max_substr << std::endl;
			auto j = locate_longest_prefix(right_max_substr);
			// if a character does not appear in the text
			// FARE RIFERIMENTO PROBLEMA CARATTERE PRESENTE NEL PATTERN MA NON NEL TESTO
			size_t b = G.LCS(pattern,i,j.first);
			////std::cout << "b: " << b << " l: " << l << std::endl;
			if(b <= l)
			{
				////std::cout << "(" << i-l << "," << l << ") " << std::endl;
				output << "(" << i-l << "," << l << ") ";
				pstart = i-l+1;
			}
			size_t f = G.LCP(pattern,i+1,j.first+1);
			////std::cout << "f: " << f << std::endl;
			i = i + f + 1;
			l = b + f;
			////std::cout << "i: " << i << " l: " << l << std::endl;
			//"j = ZFT(P[1..i])"
			//"b = LCS(P[1..i], T[1..j])"
			//if(b <= l)
				//"report (i − ℓ, i − 1)"
			//"f = LCP(P[i + 1..m], T[j + 1..n])"
			//"i = i + f + 1"
			//"l = b + f"
		}
		output << "(" << i-l << "," << l << ")" << std::endl;
	}

	void locate_fasta(std::string patternFile)
	{
		std::ifstream patterns(patternFile);
		std::ofstream output(patternFile+".mems");

		std::string line;
		size_t i=0;

    	//std::cout << "Memory peak before searching for MEMs = " <<
    	//		     malloc_count_peak() << std::endl;
		malloc_count_reset_peak();
		auto start = std::chrono::high_resolution_clock::now();

		while(std::getline(patterns, line))
		{
			if(i%2 != 0)
			{
				////std::cout << line << std::endl;
				find_MEMs(line,output);
			}
			else{ output << line << std::endl; }
			i++;
		}
    
    	std::chrono::duration<double> duration = 
    			std::chrono::high_resolution_clock::now() - start;
    	std::cout << "Memory peak while searching for MEMs = " <<
    			     malloc_count_peak() << std::endl;
    	std::cout << "Elapsed time while searching for MEMs = " <<
    			     duration.count() << std::endl;

		patterns.close();
		output.close();

		return;
	}

private:
	// prefix search data structure
	prefixSearch Z;
	// lcs/lcp data structure
	lcpDS G; // TO INCLUDE SLP index
};

}

#endif