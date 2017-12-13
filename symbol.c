#include "symbol.h"
#include "type.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct symbol* symbol_create(symbol_t kind, struct type* type, const char* name, int which, int which_total) {

	struct symbol* s = malloc(sizeof(*s));
	s->kind = kind;
	s->type = type;
	s->name = name;
	s->which = which;
	s->which_total = which_total;

	return s;

}

struct symbol* symbol_copy(struct symbol* s) {
	
	if(!s) {
		return 0;
	}

	struct symbol* new_s = malloc(sizeof(*s));
	new_s->kind = s->kind;
	new_s->type = type_copy(s->type);
	new_s->name = (s->name) ? strdup(s->name) : 0;
	new_s->which = s->which;
	new_s->which_total = s->which_total;

	return new_s;
}

void symbol_delete(struct symbol* s) {

	if (s) {

		type_delete(s->type);
		free((char*) s->name);
	
		free(s);
	}

}

const char* symbol_codegen(struct symbol* s) {

	if(s->kind == SYMBOL_GLOBAL) {
		if(s->type->kind == TYPE_STRING || s->type->kind == TYPE_ARRAY) {
			int bufsize = strlen(s->name) + 2;
			char* newbuf = malloc(sizeof(char) * bufsize);
			snprintf(newbuf, bufsize, "$%s", s->name);
			return newbuf;
		}
		else {
			return strdup(s->name);
		}
	}

	int offset = -8 * s->which_total;
	
	int bufsize = (digits_in_integer(offset) + 8);
	char* symbol_name = malloc(sizeof(char) * bufsize);
	snprintf(symbol_name, bufsize, "%d(%%rbp)", offset);

	return symbol_name;

}
