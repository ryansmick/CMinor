// scratch.h
// Header file for scratch allocation functions

#ifndef SCRATCH_H
#define SCRATCH_H

int scratch_alloc();
void scratch_free(int r);
const char* scratch_name(int r);

#endif
