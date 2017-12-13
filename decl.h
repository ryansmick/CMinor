#ifndef DECL_H
#define DECL_H

#include "type.h"
#include "expr.h"
#include "stmt.h"
#include "symbol.h"
#include <stdio.h>

struct decl {
	char *name;
	struct type *type;
	struct expr *value;
	struct stmt *code;
	int num_locals;
	struct symbol* symbol;
	struct decl *next;
};

struct decl* decl_create(char* name, struct type* type, struct expr* value, struct stmt* code, struct decl* next);
void decl_print(struct decl* d, int tabLevel);
int decl_resolve(struct decl* d, int verbose);
int decl_resolve_helper(struct decl* d, int decl_num, int total_decl_num, int verbose);
int decl_typecheck(struct decl* d);
void printTabsDecl(int tabLevel);
void decl_codegen_globals(struct decl* d, FILE* fp);
void decl_codegen(struct decl* d, FILE* fp);

#endif
