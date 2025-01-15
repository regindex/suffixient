#include <iostream>
#include <string>
#include <fstream>
#include <cstdint>
#include <vector>
#include <algorithm>

#include "suffixient_index.hpp"

int main(int argc, char* argv[])
{
    if(argc < 3)
    {
        std::cerr << "./build_index [input_text] [suffixient_set_file] OPTIONAL:" <<
                     "[supermaximal_s_lengths] [first_s_lengths] [patterns_file]" << std::endl;
        std::cerr << "Wrong number of parameters..." << std::endl;
        exit(1);
    }

    std::string inputFileName, suffixientFileName, lcsFileName, firstFileName, patternFileName;
    inputFileName = argv[1];
    suffixientFileName = argv[2];
    if(argc == 5)
    {
        lcsFileName = argv[3];
        firstFileName = argv[4];
    }
    else if(argc == 6)
    {
        lcsFileName = argv[3];
        firstFileName = argv[4];
        patternFileName = argv[5];
    }
    else{ std::cerr << "Incorrect number of parameters..." << std::endl; exit(1); }

    // initialize the index
    suffixient::suffixient_index<ctriepp::CTriePP<Lint>,lz77::LZ77_LCP_LCS_DS> index{};
    // build the index
    if(argc < 5)
        index.build(inputFileName,suffixientFileName,true);
    else
        index.build(inputFileName,suffixientFileName,lcsFileName,firstFileName,true);
    // store/load the index (not yet implemented)
    // std::cout << "tot size: " << index.store(std::string("prova.sufi")) << std::endl;
    // loaded.load("test.suff"); 
    index.locate_fasta(patternFileName);

    return 0;
}