#include "type.h"
#include "expr.h"
#include "param_list.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct type* type_create(type_t kind, struct type* subtype, struct expr* size, struct param_list* params) {
	struct type* t = malloc(sizeof(*t));

	// Add values to t
	t->kind = kind;
	t->subtype = subtype;
	t->size = size;
	t->params = params;
	t->errorless = 1;

	return t;
}

void type_print(struct type* t) {

	switch(t->kind) {
		case TYPE_INTEGER:
			printf("integer");
			break;
		case TYPE_BOOLEAN:
			printf("boolean");
			break;
		case TYPE_CHARACTER:
			printf("char");
			break;
		case TYPE_STRING:
			printf("string");
			break;
		case TYPE_ARRAY:
			printf("array [");
			expr_print(t->size, 0);
			printf("] ");
			type_print(t->subtype);
			break;
		case TYPE_FUNCTION:
			printf("function ");
			type_print(t->subtype);
			printf("(");
			param_list_print(t->params);
			printf(")");
			break;
		case TYPE_VOID:
			printf("void");
			break;
	}

}

int type_equals(struct type* a, struct type* b) {

	if(!a && !b) {
		return 1;
	}

	else if(!a || !b) {
		return 0;
	}

	if (a->kind == b->kind) {
		if (a->kind == TYPE_BOOLEAN || a->kind == TYPE_CHARACTER || a->kind == TYPE_INTEGER || a->kind == TYPE_STRING || a->kind == TYPE_VOID) {
			return 1;
		}
		else if (a->kind == TYPE_FUNCTION) {
			return type_equals(a->subtype, b->subtype) && param_list_equals(a->params, b->params);
		}
		else if (a->kind == TYPE_ARRAY) {
			return type_equals(a->subtype, b->subtype);
		}
	}

	return 0;

}

struct type* type_copy(struct type* t) {

	if(!t) {
		return 0;
	}

	struct type* new_t = malloc(sizeof(*new_t));

	new_t->kind = t->kind;
	new_t->subtype = type_copy(t->subtype);
	new_t->size = expr_copy(t->size);
	new_t->params = param_list_copy(t->params);
	new_t->errorless = t->errorless;

	return new_t;

}

void type_delete(struct type* t) {

	if(t) {
		type_delete(t->subtype);
		expr_delete(t->size);
		param_list_delete(t->params);

		free(t);
	}

}


const char* type_get_x86_type_string(struct type* t) {

	char* x86_type = malloc(sizeof(char) * 8);
	if (t->kind == TYPE_BOOLEAN || t->kind == TYPE_INTEGER || t->kind == TYPE_CHARACTER) {
		sprintf(x86_type, ".quad");
	}
	else if(t->kind == TYPE_STRING) {
		sprintf(x86_type, ".string");
	}
	else if(t->kind == TYPE_ARRAY) {
		return type_get_x86_type_string(t->subtype);
	}

	return x86_type;
}

const char* type_generate_default_literal_value(struct type* t) {

	char* return_val;
	if(!t) {
		return "";
	}

	if(t->kind == TYPE_INTEGER) {
		return_val = "0";
	}
	else if (t->kind == TYPE_CHARACTER) {
		return_val = "a";
	}
	else if (t->kind == TYPE_STRING) {
		return_val = "";
	}
	else if (t->kind == TYPE_ARRAY) {
		int size = t->size->literal_value;
		char val[2*size];
		val[0] = '0';
		int i;
		for(i = 1; i < 2*size-1; i += 2) {
			val[i] = ',';
			val[i+1] = '0';
		}
		val[2*size-1] = '\0';
		return strdup(val);
	}
	else if (t->kind == TYPE_BOOLEAN) {
		return_val = "0";
	}
	else {
		return_val = "";
	}

	return_val = strdup(return_val);
	return return_val;
}
