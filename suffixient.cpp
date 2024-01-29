// Copyright (c) 2023, Nicola Prezza.  All rights reserved.
// Use of this source code is governed
// by a MIT license that can be found in the LICENSE file.

/*
 *
 *  Created on: Dec 11, 2023
 *      Author: Nicola Prezza
 */


#include <iostream>
#include <unistd.h>
#include "internal/dna_bwt_n.hpp"
#include <stack>
#include <algorithm>
#include <sdsl/construct_bwt.hpp>
#include <sdsl/suffix_arrays.hpp>

using namespace std;
using namespace sdsl;

string input_bwt;
dna_bwt_n_t bwt;
vector<bool> suffixient_bwt; //marks set of nexessary+suffixient BWT positions

int_vector_buffer<> sa;

bool containsN = false;
uint64_t nodes=0; // number of visited nodes

uint64_t wl_leaves=0; // number of Weiner tree leaves

int last_perc = -1;
int perc = 0;
uint64_t n=0;

uint64_t rec_depth = 0;
uint64_t max_rec_depth = 0;

char TERM = '#';

void help(){

	cout << "rho [options]" << endl <<
	"Input: BWT of a DNA dataset (alphabet: A,C,G,T,N,#). Output: value of the rho repetitiveness measure and related statistics." << endl <<
	"Options:" << endl <<
	"-i <arg>    Input BWT (REQUIRED)" << endl <<
	"-t          ASCII code of the terminator. Default:" << int('#') << " (#). Cannot be the code for A,C,G,T,N." << endl;
	exit(0);
}

//recursively processes node and return cost of its subtree, i.e. total number of 
//right-extensions that we pay
uint64_t process_node(	typename dna_bwt_n_t::sa_node_t& x, 
						//The function "process_node" will add (OR) to this flag the 
						//right-extensions that are covered on node x
						flags& covered_from_wchildren
						){ 

	rec_depth++;
	max_rec_depth = std::max(max_rec_depth,rec_depth);

	uint64_t rho = 0;

	//we recurse on all but the last child of x. On the last child, we cycle in this while loop, 
	//replacing x with its last child. Since we process recursively children in decreasing order of
	//BWT interval length, this guarantees that the recursion depth is logarithmic.
	while(true){

		nodes++;
		perc = (100*nodes)/n;

		if(perc > last_perc){

			cout << perc << "%." << endl;
			last_perc = perc;

		}

		//get (right-maximal) children of x in the Weiner tree
		int t = 0;
		auto children = vector<typename dna_bwt_n_t::sa_node_t>(5); 
		bwt.get_weiner_children(x, children, t);

		if(t==0){ 

			// no children in the Weiner tree: pay all the right extensions of string(x)

			wl_leaves++;

			rho += has_right_ext_TERM(x);
			rho += has_right_ext_A(x);
			rho += has_right_ext_C(x);
			rho += has_right_ext_G(x);
			rho += has_right_ext_N(x);
			rho += has_right_ext_T(x);

			covered_from_wchildren.TM |= has_right_ext_TERM(x);
			covered_from_wchildren.A  |= has_right_ext_A(x);
			covered_from_wchildren.C  |= has_right_ext_C(x);
			covered_from_wchildren.G  |= has_right_ext_G(x);
			covered_from_wchildren.N  |= has_right_ext_N(x);
			covered_from_wchildren.T  |= has_right_ext_T(x);

			break;

		}else{

			flags tmp_covered_children {false,false,false,false,false,false};

			//scan all children but the last
			for(int i=0;i<t-1;++i)
				rho += process_node(children[i],tmp_covered_children);
			

			if(	has_right_ext_TERM(x) and 
				(not tmp_covered_children.TM) and 
				not has_right_ext_TERM(children[t-1])){

				//TERM has to be covered on node x
				covered_from_wchildren.TM = true;
				rho++;

			}

			if(	has_right_ext_A(x) and 
				(not tmp_covered_children.A) and 
				not has_right_ext_A(children[t-1])){

				//A has to be covered on node x
				covered_from_wchildren.A = true;
				rho++;

			}

			if(	has_right_ext_C(x) and 
				(not tmp_covered_children.C) and 
				not has_right_ext_C(children[t-1])){

				//C has to be covered on node x
				covered_from_wchildren.C = true;
				rho++;

			}

			if(	has_right_ext_G(x) and 
				(not tmp_covered_children.G) and 
				not has_right_ext_G(children[t-1])){

				//G has to be covered on node x
				covered_from_wchildren.G = true;
				rho++;

			}

			if(	has_right_ext_N(x) and 
				(not tmp_covered_children.N) and 
				not has_right_ext_N(children[t-1])){

				//N has to be covered on node x
				covered_from_wchildren.N = true;
				rho++;

			}

			if(	has_right_ext_T(x) and 
				(not tmp_covered_children.T) and 
				not has_right_ext_T(children[t-1])){

				//T has to be covered on node x
				covered_from_wchildren.T = true;
				rho++;

			}

			x = children[t-1];

		}
	
	}

	rec_depth--;
	return rho;

}

//input: string s, not containing 0 symbol
//output: BWT of s
string build_bwt(string& s){

	cache_config cc;
	int_vector<8> text(s.size());

	for(uint64_t i=0;i<s.size();++i) text[i] = (uint8_t)s[i];
	
	append_zero_symbol(text);
	store_to_cache(text, conf::KEY_TEXT, cc);
	construct_sa<8>(cc);
	sa = int_vector_buffer<>(cache_file_name(conf::KEY_SA, cc));

	string bwt(s.size()+1,0);

	for(uint64_t i = 0;i<s.size()+1;++i) bwt[i] = sa[i]==0 ? '#' : s[sa[i]-1];

	sdsl::remove(cache_file_name(conf::KEY_TEXT, cc));
	sdsl::remove(cache_file_name(conf::KEY_SA, cc));
	sdsl::remove(cache_file_name(conf::KEY_BWT, cc));

	return bwt;

}

int main(int argc, char** argv){

	if(argc < 3) help();

	int opt;
	while ((opt = getopt(argc, argv, "hi:o:l:t:")) != -1){
		switch (opt){
			case 'h':
				help();
			break;
			case 'i':
				input_bwt = string(optarg);
			break;
			case 't':
				TERM = atoi(optarg);
			break;
			default:
				help();
			return -1;
		}
	}

	if(TERM == 'A' or TERM == 'C' or TERM == 'G' or TERM == 'T' or TERM == 'N'){

		cout << "Error: invalid terminator '" << TERM << "'" << endl;
		help();

	}

	if(input_bwt.size()==0) help();

	cout << "Input BWT file: " << input_bwt << endl;

	cout << "Loading and indexing BWT ... " << endl;

	bwt = dna_bwt_n_t(input_bwt, TERM);

	n = bwt.size();

	cout << "Done. Size of BWT: " << n << endl;

	//navigate suffix link tree

	cout << "Starting DFS navigation of the Weiner tree." << endl;

	auto x = bwt.root();

	flags tmp_covered_children {false,false,false,false,false,false};
	uint64_t rho = process_node(x, tmp_covered_children);

	cout << "Processed " << nodes << " suffix tree nodes." << endl;
	cout << "rho = " << rho << endl;
	cout << "r = " << bwt.r() << endl;
	cout << "Number of Weiner tree leaves: " << wl_leaves << endl;
	cout << "Maximum recursion depth = " << max_rec_depth << endl;

}

