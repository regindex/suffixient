# Compute the smallest suffixient set

### Overview

Let $T[1..n]$ be a text. A set $S \subseteq {1,\dots,n}$ is $suffixient$ if any one-character extension $X\cdot a$ (string $X$ concatenated with character $a$) of any right-maximal substring $X$ of $T$ is a suffix of $T[1..i]$, for at least one position $i \in S$.

We say that a suffixient set is also $nexessary$ if no position can be removed from it without losing the suffixient property. This code computes the smallest suffixient-nexessary set of a string.

### Funding

This project has received funding from the European Research Council (ERC) under the European Union’s Horizon Europe research and innovation programme, project REGINDEX, grant agreement No 101039208

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
cat text.txt | suffixient -o output
~~~~

then the output set S is stored to file in the following format: one uint64_t storing the size $|S|$ of the set, followed by $|S|$ uint64_t storing the set itself. 

If option -o is not specified: 

~~~~
cat text.txt | suffixient
~~~~

then the output set is streamed to standard output in human-readable format.