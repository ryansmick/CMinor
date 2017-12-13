#ifndef PARAM_LIST_H
#define PARAM_LIST_H

#include "type.h"
#include "symbol.h"
#include <stdio.h>

struct param_list {
	char *name;
	struct type *type;
	struct symbol* symbol;
	struct param_list* next;
};

struct param_list* param_list_create(char* name, struct type* type, struct param_list* next);
int param_list_resolve(struct param_list* p, int verbose);
int param_list_resolve_helper(struct param_list* p, int param_num, int verbose);
int param_list_count_params(struct param_list* p);
int param_list_equals(struct param_list* a, struct param_list* b);
void param_list_print(struct param_list* p);
struct param_list* param_list_copy(struct param_list* p);
void param_list_delete(struct param_list* p);
int param_list_check_types(struct param_list* p, struct expr* e);
void param_list_save_parameters_codegen(struct param_list* p, FILE* fp);
void param_list_save_parameters_codegen_helper(struct param_list* p, FILE* fp, int param_num);
void param_list_expr_argument_list_codegen(struct expr* e, FILE* fp);
void param_list_expr_argument_list_codegen_helper(struct expr* e, FILE* fp, int arg_num);
void param_list_move_register_to_function_argument_register(int register_number, int argument_number, FILE* fp);

#endif
