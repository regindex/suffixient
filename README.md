# Compute the smallest suffixient set

### Overview

Let $T[1..n]$ be a text. A set $`S \subseteq \{1,\dots,n\}`$ is $suffixient$ if, for any one-character extension $X\cdot a$ (string $X$ concatenated with character $a$) of any right-maximal substring $X$ of $T$, there exists $i \in S$ such that $X\cdot a$ is a suffix of $T[1..i]$.

We say that a suffixient set is also $nexessary$ if no position can be removed from it without losing the suffixient property. This code computes the smallest suffixient-nexessary set of a string.

Complexity: $O(n + r\sigma)$, where $r$ is the number of equal-letter runs in the BWT of the reversed text and $\sigma$ is the alphabet size. More in detail: $O(n)$ for constructing SA and LCP arrays, then $O(n + r\sigma)$ time in one pass of those arrays using just $O(\sigma)$ words of additional memory (so very cache efficient).


### Algorithm

For every character $c$, focus on the $c$-run borders of $BWT(rev(T))$, i.e. positions $i$ such that $BWT[i-1,i] = xc$ or $BWT[i-1,i] = cx$ ($x \neq c$). Let $LCP[l,...,r]$ be the maximal substring of LCP(rev(T)) with $l \leq i \leq r$ such that $LCP[l,...,r] \geq LCP[i]$. If $BWT[l-1,...,r]$ does not contain another $c$-run border $BWT[j-1,j]$ such that $LCP[j]>LCP[i]$, then the text position corresponding to $i$ or $i-1$ (the one containing $c$ in the BWT) is inserted in the suffixient set.

Why does it work? w.l.o.g., assume $BWT[i-1,i]=xc$. If BWT position $i$ is inserted in the suffixient set, this testifies a right-maximal string $X$ of length $LCP[i]$ having right extension $c$ (i.e. $X\cdot c$ appears in the text). Moreover, the above conditions guarantee that no other right-maximal string $Y$ has an extension $Y\cdot c$ such that $X.c$ suffixes $Y.c$. This can be proved to give the smallest suffixient set because, intuitively, every suffixient set must have such a position $i'$ associated with string $X.c$.

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

for more options. The tool allows also sorting the output (option -s) and printing the size of the smallest suffixient set (option -p) and the number of runs in the BWT of the reverse text (option -r). Type "sources/suffixient_twopass for using the two pass linear algorithm".

Otherwise, you can run the smallest suffixient set construction for large texts using the PFP algorithm by typing the following command (note that at the moment you need to explicitly invert the text before running the PFP)

~~~~
python3 run_suffixient rev(text.txt)
~~~~

for the full list of options, type

~~~~
python3 run_suffixient -h
~~~~

### Funding

This project has received funding from the European Research Council (ERC) under the European Union’s Horizon Europe research and innovation programme, project REGINDEX, grant agreement No 101039208