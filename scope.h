#ifndef SCOPE_H
#define SCOPE_H

#include "hash_table.h"
#include "symbol.h"

struct scope {
	struct hash_table* variables;
	int level;
	struct scope* next;
	
};

struct scope* scope_create();
void scope_enter();
void scope_exit();
int scope_level();
void scope_bind(const char* name, struct symbol* symbol);
struct symbol* scope_lookup(const char* name);
struct symbol* scope_lookup_current(const char* name);

#endif
