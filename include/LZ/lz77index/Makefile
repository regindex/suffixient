CC = g++
CFLAGS =-O3 -std=c++14 -Wall -Wno-c++11-narrowing -DNDEBUG 
OBJS = utils.o mapper.o static_selfindex.o static_selfindex_none.o static_selfindex_lz77.o LZparser.o LZ77.o LZend.o treelist.o static_bitsequence.o static_bitsequence_brw32.o  static_bitsequence_builder_brw32.o static_permutation.o static_permutation_mrrr.o RMQ.o RMQ_succinct.o perm.o deltacodes.o patricia.o patricia_node.o bp_hb.o dfuds.o static_doublebitmap.o static_doublebitmap_s.o hash.o utils_index.o range.o static_range.o static_range_none.o static_range_chazelle.o static_sequence_wt.o wt_node.o perm2.o static_permutation_mrrr2.o static_selfindex_lzend.o directcodes.o bitrankw32int.o
LIBS = libdivsufsort.a

EXEC = $(BIN_DIR)/build_lz77 $(BIN_DIR)/load $(BIN_DIR)/extract $(BIN_DIR)/locate $(BIN_DIR)/display #$(BIN_DIR)/build_lzend $(BIN_DIR)/exist $(BIN_DIR)/locatemax
EXEC_DIR = tests
BIN_DIR = bin
LIB_DIR = lib
INCLUDE_DIR = includes

.cpp.o:
	$(CC) $(CFLAGS) -c $<

all: $(OBJS) $(LIB_DIR)/liblz77.a $(INCLUDE_DIR)

# Build static library
$(LIB_DIR)/liblz77.a: $(OBJS) | $(LIB_DIR)
	ar rcs $@ $^

# Copy headers
$(INCLUDE_DIR):
	mkdir -p $@
	cp $(wildcard *.h) $@

clean:
	rm -f core vgcore* $(OBJS) $(LIB_DIR)/liblz77.a
	rm -rf $(INCLUDE_DIR)