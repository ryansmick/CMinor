#ifndef SYMBOL_H
#define SYMBOL_H

typedef enum {
	SYMBOL_GLOBAL,
	SYMBOL_LOCAL,
	SYMBOL_PARAM
} symbol_t;

struct symbol {
	symbol_t kind;
	struct type* type;
	const char* name;
	int which;
	int which_total;
};

struct symbol* symbol_create(symbol_t kind, struct type* type, const char* name, int which, int which_total);
struct symbol* symbol_copy(struct symbol* s);
void symbol_delete(struct symbol* s);
const char* symbol_codegen(struct symbol* s);

#endif
