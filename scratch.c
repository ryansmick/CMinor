// scratch.c
// Implementation of functions in scratch_alloc.h

#include <stdio.h>
#include <stdlib.h>
#include "scratch.h"

char* scratch_names[7] = {"%rbx", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15"};
int scratch_inuse[7] = {0, 0, 0, 0, 0, 0, 0};
int scratch_length = 7;

int scratch_alloc() {

	int i = 0;
	for(i = 0; i < scratch_length; i++) {
		if(scratch_inuse[i] == 0) {
			scratch_inuse[i] = 1;
			return i;
		}
	}
	
	printf("codegen error: Out of registers\n");
	exit(1);

}

void  scratch_free(int r) {

	if(r < 0 || r >= scratch_length) {
		printf("Attempted to free register %d\n", r);
		exit(1);
	}

	scratch_inuse[r] = 0;

}

const char* scratch_name(int r) {
	if(r < 0 || r >= scratch_length) {
		printf("Attempted to retrieve name for register %d, which doesn't exist\n", r);
		exit(1);
	}

	return scratch_names[r];

}
