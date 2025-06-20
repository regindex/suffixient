# Compute the smallest suffixient set

### Overview

Let $T[1..n]$ be a text. A set $`S \subseteq \{1,\dots,n\}`$ is $suffixient$ if, for any one-character extension $X\cdot a$ (string $X$ concatenated with character $a$) of any right-maximal substring $X$ of $T$, there exists $i \in S$ such that $X\cdot a$ is a suffix of $T[1..i]$.

This software computes a smallest suffixient set of a text T.

### Install

~~~~
git clone https://github.com/regindex/suffixient
cd suffixient
mkdir build
cd build
cmake ..
make
~~~~

### Run

The tool reads its input (a text file) from standard input. If option -o is specified: 

~~~~
cat text.txt | sources/suffixient -o output
~~~~

or 

~~~~
sources/suffixient -o output < text.txt
~~~~

then the output set S is stored to file in the following format: one uint64_t storing the size $|S|$ of the set, followed by $|S|$ uint64_t storing the set itself. 

If option -o is not specified: 

~~~~
cat text.txt | sources/suffixient
~~~~

or

~~~~
sources/suffixient < text.txt
~~~~

then the output set is streamed to standard output in human-readable format.

Type

~~~~
sources/suffixient -h
~~~~

for more options. The tool allows also sorting the output (option -s) and printing the size of the smallest suffixient set (option -p) and the number of runs in the BWT of the reverse text (option -r). Type sources/one_pass or sources/fm for using two variants of the linear time algorithm".

Otherwise, you can run the smallest suffixient set construction for large repetitive texts using the PFP algorithm by typing the following command (note that in this software version you need to explicitly invert the text before running the PFP by using the -i flag).

~~~~
python3 pfp_suffixient.py -i text.txt
~~~~

for the full list of options, type

~~~~
python3 pfp_suffixient -h
~~~~

The suffixiency test receives the filename for a file containing the input text and the filename for a file containing the set being tested, which is assumed to be written as following: one uint64_t storing the size $|S|$ of the set, followed by $|S|$ uint64_t storing the set itself. Then, if input_file is the filename for the input text and input_set is the filename for the set being tested the test can be executed as:

~~~~
./test input_file input_set
~~~~

### Funding

This project has received funding from the European Research Council (ERC) under the European Union’s Horizon Europe research and innovation programme, project REGINDEX, grant agreement No. 101039208.
