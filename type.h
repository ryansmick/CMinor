#ifndef TYPE_H
#define TYPE_H

#include "decl.h"
#include "expr.h"
#include "stmt.h"
#include "param_list.h"

typedef enum {
	TYPE_BOOLEAN,
	TYPE_CHARACTER,
	TYPE_INTEGER,
	TYPE_STRING,
	TYPE_ARRAY,
	TYPE_FUNCTION,
	TYPE_VOID
} type_t;

struct type {
	type_t kind;
	struct type* subtype;
	struct expr* size;
	struct param_list* params;
	int errorless;
};

struct type* type_create(type_t kind, struct type* subtype, struct expr* size, struct param_list* params);
void type_print(struct type* t);
int type_equals(struct type* a, struct type* b);
struct type* type_copy(struct type* t);
void type_delete(struct type* t);
const char* type_get_x86_type_string(struct type* t);
const char* type_generate_default_literal_value(struct type* t);

#endif
