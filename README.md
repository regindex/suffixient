# Compute the smallest suffixient set

### Overview

Let $T[1..n]$ be a text. A set $`S \subseteq \{1,\dots,n\}`$ is $suffixient$ if, for any one-character extension $X\cdot a$ (string $X$ concatenated with character $a$) of any right-maximal substring $X$ of $T$, there exists $i \in S$ such that $X\cdot a$ is a suffix of $T[1..i]$.

We say that a suffixient set is also $nexessary$ if no position can be removed from it without losing the suffixient property. This code computes the smallest suffixient-nexessary set of a string.

This software computes the suffixient set of a text T using three different algorithms.

### Install

~~~~
git clone https://github.com/nicolaprezza/suffixient
cd suffixient
mkdir build
cd build
cmake ..
make
~~~~

### Run

The tool reads its input (a text file) from standard input. If option -o is specified: 

~~~~
cat text.txt | sources/one_pass -o output
~~~~

or 

~~~~
sources/one_pass -o output < text.txt
~~~~

then the output set S is stored to file in the following format: one uint64_t storing the size $|S|$ of the set, followed by $|S|$ uint64_t storing the set itself. 

If option -o is not specified: 

~~~~
cat text.txt | sources/one_pass
~~~~

or

~~~~
sources/one_pass < text.txt
~~~~

then the output set is streamed to standard output in human-readable format.

Type

~~~~
sources/one_pass -h
~~~~

for more options. The tool allows also sorting the output (option -s) and printing the size of the smallest suffixient set (option -p) and the number of runs in the BWT of the reverse text (option -r). Type "sources/suffixient_twopass for using the two pass linear algorithm".

Otherwise, you can run the smallest suffixient set construction for large repetitive texts using the PFP algorithm by typing the following command (note that at the moment you need to explicitly invert the text before running the PFP by typing -i flag).

~~~~
python3 run_suffixient.py -i text.txt
~~~~

for the full list of options, type

~~~~
python3 run_suffixient -h
~~~~

### Funding

This project has received funding from the European Research Council (ERC) under the European Union’s Horizon Europe research and innovation programme, project REGINDEX, grant agreement No 101039208