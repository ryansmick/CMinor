#ifndef STMT_H
#define STMT_H

#include "type.h"
#include "expr.h"
#include "decl.h"
#include <stdio.h>

struct type;

typedef enum {
	STMT_DECL,
	STMT_EXPR,
	STMT_IF_ELSE,
	STMT_FOR,
	STMT_PRINT,
	STMT_RETURN,
	STMT_BLOCK
} stmt_t;

struct stmt {
	stmt_t kind;
	struct decl* decl;
	struct expr* init_expr;
	struct expr* expr;
	struct expr* next_expr;
	struct stmt* body;
	struct stmt* else_body;
	struct stmt* next;
};

struct stmt* stmt_create(stmt_t kind, struct decl* decl, struct expr* init_expr, struct expr* expr, struct expr* next_expr, struct stmt* body, struct stmt* else_body, struct stmt* next);
void stmt_print(struct stmt* s, int tabLevel);
int stmt_resolve(struct stmt* s, int total_decl_num, int verbose, struct decl* enclosing_func);
int stmt_resolve_helper(struct stmt* s, int* decl_num, int total_decl_num, int verbose, struct decl* enclosing_func);
int stmt_typecheck(struct stmt* s, struct type* return_type);
void printTabsStmt(int tabLevel);
void stmt_codegen_globals(struct stmt* s, FILE* fp);
void stmt_codegen(struct stmt* s, FILE* fp, const char* enclosing_func_name);

#endif
