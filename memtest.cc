/*
 * memtest.cc
 *
 *  Created on: Sep 26, 2014
 *      Author: Upiter
 */

#include <stdlib.h>
#include <stdio.h>
#include <vector>

#include <Network/DebugMem.h>

struct memrec{
	void* ptr;
	size_t size;
};

std::vector<memrec> &memrecs(){
	static std::vector<memrec> mr;
	return mr;
};

signed long memused = 0;

extern "C" {

void* memt_malloc (size_t size){
	const char* dmb = DM_block;
	DM_changeBlock("memt");

	void *p = malloc(size);

	if (p!=NULL){
		memrec r = {p, size};
		memrecs().push_back(r);
		memused+=size;
	}

	DM_changeBlock(dmb);

	return p;
}

void* memt_calloc (size_t size, size_t c){
	const char* dmb = DM_block;
	DM_changeBlock("memt");

	void *p = calloc(size, c);

	if (p!=NULL){
		memrec r = {p, size};
		memrecs().push_back(r);
		memused+=size;
	}

	DM_changeBlock(dmb);

	return p;
}

void  memt_free (void* ptr){
	const char* dmb = DM_block;
	DM_changeBlock("memt");

	if (ptr!=NULL){
		free(ptr);
		for (size_t i=0; i<memrecs().size(); ++i){
			if (memrecs()[i].ptr==ptr){
				memused -= memrecs()[i].size;
				memrecs().erase(memrecs().begin()+i);
				if (memrecs().capacity()>2*memrecs().size()){
					memrecs().shrink_to_fit();
				}
				DM_changeBlock(dmb);
				return;
			}
		}
		printf("memfree - ptr not found\r\n");
	}
	DM_changeBlock(dmb);
}

}

