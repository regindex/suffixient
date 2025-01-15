/* bp_hb.cpp
 * Copyright (C) 2003, Gonzalo Navarro, all rights reserved.
 * Adapted by Rodrigo Canovas, 2010.
 * Diego Arroyuelo fixed minor details.
 *
 * Hash-based balanced parentheses definition
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
 

 /* Original note from Gonzalo Navarro:
        * I have decided not to implement Munro and Raman [SICOMP. 2001] scheme, as it is too
	* complicated and the overhead is not so small in practice. I have opted 
	* for a simpler scheme. Each open (closing) parenthesis will be able to
	* find its matching closing (open) parenthesis. If the distance is shorter
	* than b, we will do it by hand, traversing the string. Otherwise, the
	* answer will be stored in a hash table. In fact, only subtrees larger than 
	* s will have the full distance stored, while those between b and s will
	* be in another table with just log s bits. The traversal by hand proceeds
	* in fact by chunks of k bits, whose answers are precomputed.
	* Space: there cannot be more than n/s subtrees larger than s, idem b.
	* So we have (n/s)log n bits for far pointers and (n/b)log s for near 
	* pointers. The space for the table is 2^k*k*log b. The optimum s=b log n,
	* in which case the space is n/b(1 + log b + log log n) + 2^k*k*log b.
	* Time: the time is O(b/k), and we want to keep it O(log log n), so
	* k = b/log log n.
	* (previous arguments hold if there are no unary nodes, but we hope that 
	* there are not too many -- in revtrie we compress unary paths except when
	* they have id)
	* Settings: using b = log n, we have 
	* space = n log log n / log n + 2^(log n / log log n) log n
	* time = log log n
	* In practice: we use k = 8 (unsigned chars table), b = W (so work = 4 or 8)
	* and space ~= n/3 + 10 Kunsigned chars (fixed table). 
	* Notice that we need a hash table that stores only the deltas and does not
	* store the keys! (they would take log n instead of log s!). Collisions are
	* resolved as follows: see all the deltas that could be and pick the smallest
	* one whose excess is the same of the argument. To make this low we use a
	* load factor of 2.0, so it is 2n/3 after all.
	* We need the same for the reverses, for the forward is only for ('s and
	* reverses for )'s, so the proportion stays the same.
	* We also need the stream to be a bitmap to know how many open parentheses
	* we have to the left. The operations are as follows:
	* findclose: use the mechanism described above
	* findparent: similar, in reverse, looking for the current excess - 1
	* this needs us to store the (near/far) parent of each node, which may
	* cost more than the next sibling.
	* excess: using the number of open parentheses
	* enclose: almost findparent
	*/                                                                           	        

#define BITMAP_HB_HDR 64
#include "bp_hb.h"

// returns e[p..p+len-1], assuming len <= W
#define bitsW 5 // OJO

namespace lz77index{
namespace basics{


inline static unsigned int bitget_go(unsigned int *e, unsigned int p, unsigned int len){ 
	unsigned int answ;
	e += p >> bitsW; p &= (1<<bitsW)-1;
	answ = *e >> p;
	if (len == W)
	{ 
		if(p) 
			answ |= (*(e+1)) << (W-p);
	}
	else{ 
		if (p+len > W) 
			answ |= (*(e+1)) << (W-p);
		answ &= (1<<len)-1;
	}
	return answ;
}

bp_hb::bp_hb(){
	data = NULL;
	bdata = NULL;
	n = 0;                  
	sbits = 0;
	b = 0;
	sftable = NULL;
	sbtable = NULL;
	bftable = NULL;
	bbtable = NULL;
	open_sbtable = NULL;
	open_bbtable = NULL;
}


bp_hb::bp_hb(unsigned int *string, unsigned int length, unsigned int block_size, bool bwd){
	unsigned int nbits, s, near, far, pnear, pfar;
	data = string;
	n = length; 
	b = block_size;
	bdata = new static_bitsequence_brw32(string, n, 20); 
	nbits = bits(n-1);
	s = nbits*b;
	sbits = bits(s-1);
    //std::cout << "near limit: " << (1<<sbits) << std::endl;
	s = 1 << sbits; // to take the most advantage of what we can represent
	near = far = pnear = pfar = 0;
	calcsizes(~0,0,&near,&far,&pnear,&pfar);
	/*create structures for findclose()*/
	sftable = createHash(far,nbits, 1.6);
	bftable = createHash(near,sbits, 1.6);
	if(bwd){
		/*create structures for findopen()*/
		open_sbtable = createHash(far,nbits, 1.6);
		open_bbtable = createHash(near,sbits, 1.6);
		/*create structures for enclose()*/
		sbtable = createHash(pfar,nbits, 1.8);
		bbtable = createHash(pnear,sbits, 1.8);
	}
	else{ 
		sbtable = bbtable = open_sbtable = open_bbtable = NULL;
	}
	filltables(~0,0,bwd);
	/*compute tables*/
	for (int i=0;i<256;i++) {
			fcompchar(i,FwdPos[i],Excess+i);
			bcompchar(i,BwdPos[i]);
	}
}


/* creates a parentheses structure from a bitstring, which is shared
 * n is the total number of parentheses, opening + closing
 * */
unsigned int bp_hb::calcsizes(unsigned int posparent, unsigned int posopen, unsigned int *near, unsigned int *far, unsigned int *pnear, unsigned int *pfar){
	unsigned int posclose,newpos;
	if ((posopen == n) || bitget(data,posopen))
		return posopen; // no more trees
	newpos = posopen;
	do{ 
		posclose = newpos+1;
		newpos = calcsizes (posopen,posclose,near,far,pnear,pfar);
	}
	while (newpos != posclose);
	
	if ((posclose < n) && (posclose-posopen > b)){ // exists and not small
		if ((int)(posclose-posopen) < (1<<sbits)) 
			(*near)++; // near pointer
		else 
			(*far)++;
	}
	if ((posopen > 0) && (posopen-posparent > b)){ // exists and not small
		if ((int)(posopen-posparent) < (1<<sbits)) 
			(*pnear)++; // near pointer
		else 
			(*pfar)++;
	}
	return posclose;
}

unsigned int bp_hb::filltables(unsigned int posparent, unsigned int posopen, bool bwd){
	unsigned int posclose,newpos;
	if ((posopen == n) || bitget(data,posopen))
		return posopen; // no more trees
	newpos = posopen;
	do{ 
		posclose = newpos+1;
		newpos = filltables(posopen,posclose,bwd);
	}
	while (newpos != posclose);
	if ((posclose < n) && (posclose-posopen > b)) { // exists and not small
		if ((int)(posclose-posopen) < (1<<sbits)) { // near pointers
			insertHash(bftable,posopen,posclose-posopen);
			if (open_bbtable)
				insertHash(open_bbtable,posclose,posclose-posopen);
		}
		else { // far pointers
			insertHash(sftable,posopen,posclose-posopen); 
			if (open_sbtable)
				insertHash(open_sbtable,posclose,posclose-posopen);
		}
	}
	if (bwd && (posopen > 0) && (posopen-posparent > b)) { //exists and not small
		if ((int)(posopen-posparent) < (1<<sbits)) // near pointer
			insertHash(bbtable,posopen,posopen-posparent);
		else // far pointers
			insertHash(sbtable,posopen,posopen-posparent);
	}
	return posclose;
}

void bp_hb::fcompchar(unsigned char x, unsigned char *pos, char *excess){
	int exc = 0;
	unsigned int i;
	for(i=0;i<W/2;i++) 
		pos[i] = 0;
	for(i=0;i<8;i++){
		if (x & 1) { // closing
			exc--; 
			if ((exc < 0) && !pos[-exc-1]) 
				pos[-exc-1] = i+1;
		}
		else 
			exc++;
		x >>= 1;
	}
	*excess = exc;
}

void bp_hb::bcompchar (unsigned char x, unsigned char *pos){
	int exc = 0;
	unsigned int i;
	for (i=0;i<W/2;i++) 
		pos[i] = 0;
	for (i=0;i<8;i++){
		if (x & 128) { // opening, will be used on complemented masks
			exc++; 
			if ((exc > 0) && !pos[exc-1]) 
				pos[exc-1] = i+1;
		}
		else 
			exc--;
		x <<= 1;
	}
}

bp_hb::~bp_hb(){
	delete bdata;
	if(sftable != NULL)
        destroyHash(sftable);
	if(sbtable != NULL) 
		destroyHash(sbtable);
	if(bftable != NULL)
        destroyHash(bftable);
	if(bbtable != NULL) 
		destroyHash(bbtable);
	if(open_sbtable != NULL)
        destroyHash(open_sbtable);
    if(open_bbtable != NULL)	
        destroyHash(open_bbtable);
}

/* # open - # close at position i, not included*/
unsigned int bp_hb::excess(int v){
	return v - 2*bdata->rank1(v)+1; 
}

/* the position of the closing parenthesis corresponding to (opening)
 * parenthesis at position i
 */

unsigned int bp_hb::close(int v){
    if(v==0)return n-1;
	unsigned int bitW;
	unsigned int len,res,minres,exc;    
	unsigned int len_aux;
	unsigned int pos = v;
	int cont;
	unsigned char _W1;
	handle h;
	unsigned int myexcess;
	//first see if it is at small distance
	len = b; 
	if (v+len >= n) 
		len = n-v-1;
	len_aux = len;
	exc = 0; 
	len = 0;
	while((int)len_aux > 0){
		if((int)len_aux < 32)
			bitW = bitget_go(data, pos+1,len_aux);
		else
			bitW = bitget_go(data, pos+1, 32);
		cont = 0 ;
		while(bitW && (exc < b/2)) {
			//either we shift it all or it only opens parentheses or too open parentheses
			_W1 = bitW & 255;
			if(exc < W/2){
				if ((int)(res = FwdPos[_W1][exc])) 
					return v+len+res;
			}
			bitW >>= 8; 
			exc += Excess[_W1];
			len += 8;
			cont ++;
		}
		if(cont != 4){
			_W1 = 0;
			while(cont!=4){
				exc += Excess[_W1];
				len +=8;
				cont++;
			}
		}
		pos +=32;
		len_aux = len_aux-32;
	}
	// ok, it's not a small distance, try with hashing btable
	minres = 0;
	myexcess = excess(v-1);
	res = searchHash(bftable,v,&h);
	while (res) {
		if (!minres || (res < minres)) 
			if ((v+res+1 < n) && (excess(v+res) == myexcess)) 
				minres = res;
		res = nextHash(bftable,&h);
	}
	if (minres) 
		return v+minres;
	// finally, it has to be a far pointer
	res = searchHash(sftable,v,&h);
	while (res) {
		if (!minres || (res < minres)){
			if ((v+res+1 < n) && (excess(v+res) == myexcess))
				minres = res;
		}
		res = nextHash(sftable,&h);
	}

	return v+minres; //there should be one if the sequence is balanced!
}


unsigned int bp_hb::open(int v){
	unsigned int bitW;
	unsigned int len,res,minres,exc;
	unsigned int len_aux;
	unsigned int pos = v;
	int cont;
	unsigned char _W1;
	handle h;
	unsigned int myexcess;
	/*first see if it is at small distance*/
	len = b; 
	if ((unsigned int)v < len) 
		len = v-1;
	len_aux = len;
	exc = 0; 
	len = 0;
	while((int)len_aux > 0){
		if((int)len_aux < 32)
			bitW = ~bitget_go(data,pos-len_aux,len_aux) << (W-len_aux);
		else
			bitW = ~bitget_go(data,pos-W,W);
		cont = 0;
		while (bitW && (exc < b/2)) {
			/*either we shift it all or it only closes parentheses or to
			 * many closed parentheses*/
			_W1 = (bitW >> (W-8));
			if(exc < W/2){
				if ((res = BwdPos[_W1][exc])) 
					return v-len-res;
			}
			bitW <<= 8; 
			exc += Excess[_W1];  /*note _W1 is complemented!*/
			len += 8;
			cont++;
		}		
		if(cont != 4){
			_W1=0;
			while(cont!=4){
				exc += Excess[_W1];
				len +=8;
				cont++;
			}
		}
		pos-=32;
		len_aux -=32;
	}			
	/* ok, it's not a small distance, try with hashing btable*/
	minres = 0;
	myexcess = excess(v-1)-1;
	res = searchHash(open_bbtable,v,&h);
	while (res) {
		if (!minres || (res < minres)) 
			if (((int)(v-res) >= 0) && (excess(v-res-1) == myexcess)) 
				minres = res;
		res = nextHash(open_bbtable,&h);
	}
	if (minres) 
		return v-minres;
	/* finally, it has to be a far pointer*/
	res = searchHash(open_sbtable,v,&h);
	while (res) {
		if (!minres || (res < minres))
			if (((int)(v-res) >= 0) && (excess(v-res-1) == myexcess))
				minres = res;
		res = nextHash(open_sbtable,&h);
	}
	return v-minres; /*there should be one if the sequence is balanced!*/
}

unsigned int bp_hb::enclose(int v){
	if (v == 0) 
		return (unsigned int)-1; // no parent!
	return parent(v);
}

unsigned int bp_hb::parent(int v){
	unsigned int bitW;
	unsigned int len,res,minres,exc;
	unsigned int len_aux;
	unsigned int pos = v;
	int cont;
	unsigned char _W1;
	handle h;
	unsigned int myexcess;
	/* first see if it is at small distance*/
	len = b; 
	if ((unsigned int)v < len) 
		len = v;//original = v-1
	len_aux = len;
	exc = 0; 
	len = 0; 
	do{
		if((int)len_aux < 32)
			bitW = ~bitget_go(data,pos-len_aux,len_aux) << (W-len_aux);
		else
			bitW = ~bitget_go(data,pos-W,W);
		cont = 0;
		while (bitW && (exc < b/2)) {
			/* either we shift it all or it only closes parentheses or too
			 * many closed parentheses*/
			_W1 = (bitW >> (W-8));
			if(exc < W/2){
				if ((res = BwdPos[_W1][exc]))
					return v-len-res;
			}
			bitW <<= 8; 
			exc += Excess[_W1]; // note _W1 is complemented!
			len += 8;
			cont ++;
		} 
		if(cont != 4){
			_W1=0;
			while(cont!=4){
				exc += Excess[_W1];
				len +=8;
				cont++;
			}
		}
		pos -= 32;
		len_aux -= 32;
	}	
	while((int)len_aux > 0);
	/* ok, it's not a small distance, try with hashing btable*/
	minres = 0;
	myexcess = excess(v-1) - 1;
	res = searchHash(bbtable,v,&h);
	while (res) {
		if (!minres || (res < minres)) 
			if (((int)v-(int)res >= 0) && (excess(v-res-1) == myexcess)) 
				minres = res;
		res = nextHash(bbtable,&h);
	}
	if (minres) 
		return v-minres;
	/*finally, it has to be a far pointer*/
	res = searchHash(sbtable,v,&h);
	while (res) {
		if (!minres || (res < minres)) 
			if (((int)v-(int)res >= 0) && (excess(v-res-1) == myexcess))
				minres = res;
		res = nextHash(sbtable,&h);
	}
	return v-minres;  /*there should be one if the sequence is balanced!*/
}


unsigned int bp_hb::size(){
	unsigned int mem = sizeof(bp_hb);
	mem += bdata->size();
	mem += sizeofHash(sftable);
	mem += sizeofHash(sbtable);
	mem += sizeofHash(bftable);
	mem += sizeofHash(bbtable);
	mem += sizeofHash(open_sbtable);
	mem += sizeofHash(open_bbtable); 
	mem += 2*sizeof(unsigned char)*256*W/2;
	mem += sizeof(char)*256;
	return mem;
}

int bp_hb::save(FILE *fp){
	unsigned int wr = BITMAP_HB_HDR;
	if(fwrite(&wr,sizeof(unsigned int),1,fp)!=1){
        return 1;
    }
    if(fwrite(&n,sizeof(unsigned int),1,fp)!=1){
        return 2;
    }
    if(fwrite(&sbits,sizeof(unsigned int),1,fp)!=1){
        return 3;
    }
	if(fwrite(&b,sizeof(unsigned int),1,fp)!=1){
        return 4;
    }
	if(bdata->save(fp) != 0){
		return 5;
	}
    /*TODO modify saveHash to return a value*/
	saveHash(fp, sftable);
	saveHash(fp, sbtable);
	saveHash(fp, bftable);	
	saveHash(fp, bbtable);	
	saveHash(fp, open_sbtable);
	saveHash(fp, open_bbtable);
	return 0;
}

bp_hb* bp_hb::load(FILE *fp){
	bp_hb* new_bp = new bp_hb();
    unsigned int hdr;     
    if(fread(&hdr, sizeof(unsigned int),1,fp)!=1 || hdr != BITMAP_HB_HDR){
        delete new_bp;
        return NULL;
    }
    if(fread(&new_bp->n,sizeof(unsigned int),1,fp)!=1){
        delete new_bp;
        return NULL;
    }
	if(fread(&new_bp->sbits,sizeof(unsigned int),1,fp)!=1){
        delete new_bp;
        return NULL;
    }
	if(fread(&new_bp->b,sizeof(unsigned int),1,fp)!=1){
        delete new_bp;
        return NULL;
    }
	new_bp->bdata = static_bitsequence::load(fp);
    if(new_bp->bdata==NULL){
        std::cout<<"BDATA is NULL"<<std::endl;
        delete new_bp;
        return NULL;
    }
	new_bp->data = ((static_bitsequence_brw32 *)new_bp->bdata)->data;
	new_bp->sftable = loadHash(fp);
	new_bp->sbtable = loadHash(fp);
	new_bp->bftable = loadHash(fp);
	new_bp->bbtable = loadHash(fp);
	new_bp->open_sbtable = loadHash(fp);
	new_bp->open_bbtable = loadHash(fp);
	/*compute tables*/
	for (int i=0;i<256;i++) {
		new_bp->fcompchar(i,new_bp->FwdPos[i],new_bp->Excess+i);
		new_bp->bcompchar(i,new_bp->BwdPos[i]);
	}
	return new_bp;
}

}
}

