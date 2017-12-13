#include "decl.h"
#include "type.h"
#include "stmt.h"
#include "symbol.h"
#include "scope.h"
#include "param_list.h"
#include "scratch.h"
#include <stdlib.h>
#include <stdio.h>

// Function to create a struct decl (as part of the abstract syntax tree) and return it
struct decl* decl_create(char* name, struct type* type, struct expr* value, struct stmt* code, struct decl* next) {
	struct decl* d = malloc(sizeof(*d));

	// Add values to d
	d->name = name;
	d->type = type;
	d->value = value;
	d->code = code;
	d->num_locals = 0;
	d->next = next;

	return d;
}

void decl_print(struct decl* d, int tabLevel) {

	// If d is null, don't print anything and return
	if(!d) {
		return;
	}

	// Print tabs
	printTabsDecl(tabLevel);

	// Print identifier
	printf("%s: ", d->name);

	type_print(d->type);

	// Print assignment statement
	if (d->value) {
		printf(" = ");

		expr_print(d->value, 0);

		printf(";\n");
	}

	// Print function body
	else if (d->code) {
		printf(" = {\n");
		stmt_print(d->code, tabLevel + 1);

		printTabsDecl(tabLevel);
		printf("}\n");
	}
	else {
		printf(";\n");
	}

	// Print the next decl in the list
	decl_print(d->next, tabLevel);

}

int decl_resolve(struct decl* d, int verbose) {
	return decl_resolve_helper(d, 1, 1, verbose);
}

// decl_resolve helper function allowing for passing the local declaration number
// Returns whether the resolving was successful or not
int decl_resolve_helper(struct decl* d, int decl_num, int total_decl_num, int verbose) {
	if (!d) {
		return 1;
	}

	// Set the result to true
	int result = 1;

	int isGlobal = scope_level() == 1;
	struct symbol* s = symbol_create(
			(isGlobal) ? SYMBOL_GLOBAL : SYMBOL_LOCAL,
			d->type,
			d->name,
			(isGlobal) ? 0 : decl_num,
			(isGlobal) ? 0 : total_decl_num
			);

	// Check if the variable is already declared in the current scope
	struct symbol* prev = scope_lookup_current(d->name);
	if(prev && !(d->type->kind == TYPE_FUNCTION && type_equals(d->type, prev->type))) {
		printf("resolve error: Redeclaration of variable \"%s\" (", d->name);
		type_print(d->type);
		printf("). Previous declaration was of type (");
		type_print(prev->type);
		printf(").\n");
		result = 0;
	}

	// Bind the variable to the scope
	scope_bind(d->name, s);

	// Add the symbol to the decl struct
	d->symbol = s;

	// Resolve the expression and check for errors
	if (d->value) {
		result = expr_resolve(d->value, verbose) && result;
	}

	// Resolve a function body and check for errors
	if(d->code) {
		scope_enter();
		result = param_list_resolve(d->type->params, verbose) && result;
		total_decl_num += param_list_count_params(d->type->params);
		result = stmt_resolve(d->code, total_decl_num, verbose, d) && result;
		scope_exit();
	}

	// Resolve the next decl and check for errors
	result = decl_resolve_helper(d->next, (isGlobal) ? decl_num : decl_num + 1,  (isGlobal) ? total_decl_num : total_decl_num + 1, verbose) && result;

	// Return whether or not the function executed successfully
	return result;
}

int decl_typecheck(struct decl* d) {

	if(!d) {
		return 1;
	}

	int result = 1;

	if (d->value) {
		struct type* right_type = expr_typecheck(d->value);
		if(!type_equals(d->type, right_type)) {
			printf("type error: attempted to assign value of type ");
			type_print(right_type);
			printf(" (");
			expr_print(d->value, 0);
			printf(") to variable of type ");
			type_print(d->type);
			printf(" (%s)\n", d->name);
			result = 0;
		}
		else if (right_type->kind == TYPE_ARRAY) {
			if(d->symbol->kind == SYMBOL_GLOBAL) {
				// size must match initialization list length
				if(d->type->size->kind != EXPR_INTEGER_LITERAL) {
					printf("type error: global array %s must have constant size, not ", d->name);
					expr_print(d->type->size, 0);
					printf("\n");
					result = 0;
				}	
				else if (d->value->kind != EXPR_ARRAY_INITIALIZER) {
					printf("type error: global array %s must be initialized with a constant value, not ", d->name);
					expr_print(d->value, 10);
					printf("\n");
					result = 0;
				}
				else {
					if (d->type->size->literal_value != right_type->size->literal_value) {
						printf("type error: global array %s with size %d cannot be initialized to size %d (", d->name, d->type->size->literal_value, right_type->size->literal_value);
						expr_print(d->value, 0);
						printf(")\n");
						result = 0;
					}
	
					if (!expr_list_all_constants(d->type->subtype, d->value->right)) {
						result = 0;
					}
				}
			}
			else {	
				if (d->value->kind == EXPR_ARRAY_INITIALIZER) {
					printf("type error: cannot initialize local array %s with element list ", d->name);
					expr_print(d->value, 0);
					printf("\n");
					result = 0;
				}
				else if (d->type->size && right_type->size && d->type->size->literal_value != right_type->size->literal_value) {
					printf("type error: cannot assign array ");
					expr_print(d->value, 0);
					printf(" of size %d to array %s of size %d\n", d->type->size->literal_value, d->name, right_type->size->literal_value);
					result = 0;
				}
			}

		} 		

		if(!right_type->errorless) {
			result = 0;
		}
		type_delete(right_type);
	}

	if (d->code) {
		result = stmt_typecheck(d->code, d->type->subtype) && result;
	}

	result = decl_typecheck(d->next) && result;

	return result;
}

void printTabsDecl(int tabLevel) {
	int i;
	for(i = 0; i < tabLevel; i++) {
		printf("\t");
	}
}

void decl_codegen_globals(struct decl* d, FILE* fp) {

	// If d is null, don't write anything and return
	if(!d) {
		return;
	}

	if (d->symbol->kind == SYMBOL_GLOBAL) {
		if(d->type->kind == TYPE_BOOLEAN || d->type->kind == TYPE_INTEGER || d->type->kind == TYPE_CHARACTER || d->type->kind == TYPE_STRING || d->type->kind == TYPE_ARRAY) {
			const char* x86_type = type_get_x86_type_string(d->type);
			const char* literal_value;
			if(d->value) {
				literal_value = expr_get_literal_value(d->value);
			}
			else {
				literal_value = type_generate_default_literal_value(d->type);
			}
			fprintf(fp, "%s: %s %s\n", d->name, x86_type, literal_value);
			free((char*) x86_type);
			free((char*) literal_value);
		}
		else if (d->type->kind == TYPE_FUNCTION) {
			stmt_codegen_globals(d->code, fp);
		}

	}
	else {
		expr_codegen_globals(d->value, fp);
	}

	// Print the next decl in the list
	decl_codegen_globals(d->next, fp);


}

void decl_codegen(struct decl* d, FILE* fp) {

	// If d is null, don't write anything and return
	if(!d) {
		return;
	}

	if (d->symbol->kind == SYMBOL_GLOBAL) {
		if ( d->type->kind == TYPE_FUNCTION && d->code) {
			// Set up function label
			fprintf(fp, ".globl %s\n", d->name);
			fprintf(fp, "%s:\n", d->name);

			// Set up stack frame
			fprintf(fp, "PUSHQ %%rbp\n");
			fprintf(fp, "MOVQ %%rsp, %%rbp\n");

			// Store function parameters
			param_list_save_parameters_codegen(d->type->params, fp);

			// Allocate space for local variables
			fprintf(fp, "SUBQ $%d, %%rsp\n", (8 * d->num_locals));

			// Store callee-saved function parameters
			fprintf(fp, "PUSHQ %%rbx\n");
			fprintf(fp, "PUSHQ %%r12\n");
			fprintf(fp, "PUSHQ %%r13\n");
			fprintf(fp, "PUSHQ %%r14\n");
			fprintf(fp, "PUSHQ %%r15\n");
		
			// Generate code for statements in function	
			stmt_codegen(d->code, fp, d->name);

			// Emit epilogue label
			fprintf(fp, "%s_epilogue:\n", d->name);
			
			// Restore callee saved registers
			fprintf(fp, "POPQ %%r15\n");
			fprintf(fp, "POPQ %%r14\n");
			fprintf(fp, "POPQ %%r13\n");
			fprintf(fp, "POPQ %%r12\n");
			fprintf(fp, "POPQ %%rbx\n");
		
			// Reset stack frame
			fprintf(fp, "MOVQ %%rbp, %%rsp\n");
			fprintf(fp, "POPQ %%rbp\n");

			// Return to caller
			fprintf(fp, "ret\n");
		}
	}
	else {
		if (d->type->kind != TYPE_ARRAY) {
			if (d->value) {
				// Generate code for RHS
				expr_codegen(d->value, fp);

				// Write rhs to stack
				const char* var_loc = symbol_codegen(d->symbol);
				fprintf(fp, "MOVQ %s, %s\n", scratch_name(d->value->register_number), var_loc);

				// Free var_loc
				free((char*) var_loc);

				// Free register
				scratch_free(d->value->register_number);
			}
		}
		else {
			printf("codegen error: local arrays not implemented\n");
			exit(1);
		}
	}

	// Print the next decl in the list
	decl_codegen(d->next, fp);


}
