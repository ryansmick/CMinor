#ifndef EXPR_H
#define EXPR_H

#include "type.h"
#include "stmt.h"
#include "decl.h"
#include "symbol.h"
#include <stdio.h>

typedef enum {
	EXPR_FALSE,
	EXPR_TRUE,
	EXPR_PLUS,
	EXPR_MINUS,
	EXPR_UNARY_MINUS,
	EXPR_NOT,
	EXPR_XOR,
	EXPR_MULT,
	EXPR_DIVIDE,
	EXPR_MODULUS,
	EXPR_LT,
	EXPR_GT,
	EXPR_ASSIGN,
	EXPR_AND,
	EXPR_OR,
	EXPR_INCREMENT,
	EXPR_DECREMENT,
	EXPR_EQUAL,
	EXPR_GE,
	EXPR_LE,
	EXPR_NE,
	EXPR_INTEGER_LITERAL,
	EXPR_STRING_LITERAL,
	EXPR_CHAR_LITERAL,
	EXPR_SUBSCRIPT,
	EXPR_NAME,
	EXPR_CALL,
	EXPR_ARRAY_INITIALIZER
} expr_t;

struct expr {
	expr_t kind;
	int precedence;
	struct expr* left;
	struct expr* right;
	const char* name;
	struct symbol* symbol;
	int literal_value;
	const char* string_literal;
	const char* original_literal_value;
	int register_number;
	const char* global_name;
	struct expr* next;
};

struct expr* expr_create(expr_t kind, int precedence, struct expr* left, struct expr* right, const char* name, int literal_value, const char* string_literal, const char* original_literal_value);
void expr_print(struct expr* e, int parentPrecedence);
void expr_print_single(struct expr* e);
void expr_print_operator(expr_t e);
int expr_resolve(struct expr* e, int verbose);
struct expr* expr_copy(struct expr* e);
void expr_delete(struct expr* e);
struct type* expr_typecheck(struct expr* e);
int expr_list_all_constants(struct type* t, struct expr* e);
const char* expr_get_literal_value(struct expr* e);
void expr_codegen_globals(struct expr* e, FILE* fp);
const char* expr_generate_string_global_name();
char* translate_expr_t_to_string(expr_t num);
void expr_codegen(struct expr* e, FILE* fp);
int expr_call_function_codegen(const char* function_name, FILE* fp);

#endif
