#include "param_list.h"
#include "type.h"
#include "scope.h"
#include "symbol.h"
#include "scratch.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int param_list_max_params = 6;
char* param_list_param_registers[6] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};

// Create and return param list struct
struct param_list* param_list_create(char* name, struct type* type, struct param_list* next) {
	struct param_list* p = malloc(sizeof(*p));

	// Add values to p
	p->name = name;
	p->type = type;
	p->next = next;

	return p;
}

int param_list_resolve(struct param_list* p, int verbose) {
	return param_list_resolve_helper(p, 1, verbose);
}

int param_list_resolve_helper(struct param_list* p, int param_num, int verbose) {
	if (!p) {
		return 1;
	}

	int result = 1;

	struct symbol* s = symbol_create(
			SYMBOL_PARAM,
			p->type,
			p->name,
			param_num,
			param_num
			);

	// Check if the variable is already declared in the current scope
	struct symbol* prev = scope_lookup_current(p->name);
	if(prev) {
		printf("resolve error: Redeclaration of variable \"%s\" (", p->name);
		type_print(p->type);
		printf("). Previous declaration was of type (");
		type_print(prev->type);
		printf(").\n");
		result = 0;
	}

	// Bind the variable to the scope
	scope_bind(p->name, s);

	// Add the symbol to the decl struct
	p->symbol = s;

	return param_list_resolve_helper(p->next, param_num + 1, verbose) && result;

}

int param_list_count_params(struct param_list* p) {

	if(!p) {
		return 0;
	}

	return param_list_count_params(p->next) + 1;

}

// Determine if two param lists are equal
int param_list_equals(struct param_list* a, struct param_list* b) {

	if (!a && !b) {
		return 1;
	}

	else if (!a || !b) {
		return 0;
	}

	return type_equals(a->type, b->type) && param_list_equals(a->next, b->next);

}

void param_list_print(struct param_list* p) {

	if(!p) {
		return;
	}

	printf("%s: ", p->name);
	type_print(p->type);

	if(p->next) {
		printf(", ");
		param_list_print(p->next);
	}

}

struct param_list* param_list_copy(struct param_list* p) {

	if (!p) {
		return 0;
	}

	struct param_list* new_p = malloc(sizeof(*new_p));

	new_p->name = strdup(p->name);
	new_p->type = type_copy(p->type);
	new_p->symbol = symbol_copy(p->symbol);
	new_p->next = param_list_copy(p->next);

	return new_p;
}

void param_list_delete(struct param_list* p) {

	if (!p) {
		return;
	}

	free(p->name);
	type_delete(p->type);
	symbol_delete(p->symbol);
	param_list_delete(p->next);

	free(p);

}

int param_list_check_types(struct param_list* p, struct expr* e) {

	if (!p && !e) {
		return 1;
	}

	else if (!p) {
		printf("type error: extra expression given in function call (");
		expr_print_single(e);
		printf(")\n");
		return 0;
	}

	else if (!e) {
		printf("type error: parameter of type ");
		type_print(p->type);
		printf(" missing from function call\n");
		return 0;

	}

	int result = 1;

	struct type* t = expr_typecheck(e);
	if(!type_equals(p->type, t)) {
		printf("type error: expression of type ");
		type_print(t);
		printf(" (");
		expr_print_single(e);
		printf(") does not match parameter of type ");
		type_print(p->type);
		printf("\n");
		result = 0;
	}

	return param_list_check_types(p->next, e->next) && result;


}


void param_list_save_parameters_codegen(struct param_list* p, FILE* fp) {

	param_list_save_parameters_codegen_helper(p, fp, 0);

}

void param_list_save_parameters_codegen_helper(struct param_list* p, FILE* fp, int param_num) {

	if(!p) {
		return;
	}

	if (param_num >= param_list_max_params) {
		printf("codegen error: too many arguments. Functions may not take more than %d arguments\n", param_list_max_params);
		exit(1);
	}

	fprintf(fp, "PUSHQ %s\n", param_list_param_registers[param_num]);
	
	// Recurse to next item in list
	param_list_save_parameters_codegen_helper(p->next, fp, param_num + 1);

}

void param_list_expr_argument_list_codegen(struct expr* e, FILE* fp) {
	param_list_expr_argument_list_codegen_helper(e, fp, 0);
}

void param_list_expr_argument_list_codegen_helper(struct expr* e, FILE* fp, int arg_num) {

	if (!e) {
		return;
	}

	// Base case: error out if not enough registers
	if (arg_num >= param_list_max_params) {
		printf("codegen error: too many arguments. Functions may not take more than %d arguments\n", param_list_max_params);
		exit(1);
	}

	// Generate code for current expression
	expr_codegen(e, fp);
	
	// Recurse on the next item in the list
	param_list_expr_argument_list_codegen_helper(e->next, fp, arg_num + 1);

	// Place value from expression into correct register
	param_list_move_register_to_function_argument_register(e->register_number, arg_num, fp);

	// Free the scratch register
	scratch_free(e->register_number);

}

void param_list_move_register_to_function_argument_register(int register_number, int argument_number, FILE* fp) {

	fprintf(fp, "MOVQ %s, %s\n", scratch_name(register_number), param_list_param_registers[argument_number]);

}
