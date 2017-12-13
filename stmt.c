#include "stmt.h"
#include "decl.h"
#include "expr.h"
#include "type.h"
#include "scratch.h"
#include "scope.h"
#include "label.h"
#include <stdlib.h>
#include <stdio.h>

// Function to construct a statement struct and return it
struct stmt* stmt_create(stmt_t kind, struct decl* decl, struct expr* init_expr, struct expr* expr, struct expr* next_expr, struct stmt* body, struct stmt* else_body, struct stmt* next) {
	struct stmt* s = malloc(sizeof(*s));

	// Add values to s
	s->kind = kind;
	s->decl = decl;
	s->init_expr = init_expr;
	s->expr = expr;
	s->next_expr = next_expr;
	s->body = body;
	s->else_body = else_body;
	s->next = next;

	return s;
}

void stmt_print(struct stmt* s, int tabLevel) {

	if(!s) {
		return;
	}

	switch(s->kind) {
		case STMT_DECL:
			decl_print(s->decl, tabLevel);
			break;
		case STMT_EXPR:
			printTabsStmt(tabLevel);
			expr_print(s->expr, 0);
			printf(";\n");
			break;
		case STMT_IF_ELSE:
			printTabsStmt(tabLevel);
			printf("if (");
			expr_print(s->expr, 0);
			printf(")\n");
			stmt_print(s->body, s->body->kind == STMT_BLOCK ? tabLevel : tabLevel + 1);

			// If the if statement has an else block, print that too
			if(s->else_body) {
				printTabsStmt(tabLevel);
				printf("else\n");
				stmt_print(s->else_body, s->body->kind == STMT_BLOCK ? tabLevel : tabLevel + 1);
			}
			break;
		case STMT_BLOCK:
			printTabsStmt(tabLevel);
			printf("{\n");
			stmt_print(s->body, tabLevel + 1);
			printTabsStmt(tabLevel);
			printf("}\n");
			break;
		case STMT_FOR:
			printTabsStmt(tabLevel);
			printf("for(");
			expr_print(s->init_expr, 0);
			printf(";");
			expr_print(s->expr, 0);
			printf(";");
			expr_print(s->next_expr, 0);
			printf(")\n");
			stmt_print(s->body, s->body->kind == STMT_BLOCK ? tabLevel : tabLevel + 1);
			break;
		case STMT_PRINT:
			printTabsStmt(tabLevel);
			printf("print ");
			expr_print(s->expr, 0);
			printf(";\n");
			break;
		case STMT_RETURN:
			printTabsStmt(tabLevel);
			printf("return ");
			expr_print(s->expr, 0);
			printf(";\n");
			break;
	}

	stmt_print(s->next, tabLevel);

}

int stmt_resolve(struct stmt* s, int total_decl_num, int verbose, struct decl* enclosing_func) {
	int decl_num = 1;
	return stmt_resolve_helper(s, &decl_num, total_decl_num, verbose, enclosing_func);
}

int stmt_resolve_helper(struct stmt* s, int* decl_num, int total_decl_num, int verbose, struct decl* enclosing_func) {

	if (!s) {
		return 1;
	}

	int result = 1;

	switch(s->kind) {
		case STMT_DECL:
			result = decl_resolve_helper(s->decl, *decl_num, total_decl_num, verbose) && result;
			(*decl_num) += 1;
			enclosing_func->num_locals++;
			enclosing_func->symbol->which_total += 1;
			total_decl_num++;
			break;
		case STMT_EXPR:
			result = expr_resolve(s->expr, verbose) && result;
			break;
		case STMT_IF_ELSE:
			result = expr_resolve(s->expr, verbose) && result;
			result = stmt_resolve_helper(s->body, decl_num, total_decl_num, verbose, enclosing_func) && result;

			// If the if statement has an else block, resolve that too
			if(s->else_body) {
				result = stmt_resolve_helper(s->else_body, decl_num, total_decl_num, verbose, enclosing_func) && result;
			}
			break;
		case STMT_BLOCK:
			scope_enter();
			result = stmt_resolve_helper(s->body, decl_num, total_decl_num, verbose, enclosing_func) && result;
			scope_exit();
			break;
		case STMT_FOR:
			result = expr_resolve(s->init_expr, verbose) && result;
			result = expr_resolve(s->expr, verbose) && result;
			result = expr_resolve(s->next_expr, verbose) && result;
			result = stmt_resolve_helper(s->body, decl_num, total_decl_num, verbose, enclosing_func) && result;
			break;
		case STMT_PRINT:
		case STMT_RETURN:
			result = expr_resolve(s->expr, verbose) && result;
			break;
	}

	return stmt_resolve_helper(s->next, decl_num, total_decl_num, verbose, enclosing_func) && result;


}

int stmt_typecheck(struct stmt* s, struct type* return_type) {

	if (!s) {
		return 1;
	}

	int result = 1;

	switch(s->kind) {
		case STMT_DECL:
			result = decl_typecheck(s->decl) && result;
			break;
		case STMT_EXPR:
			;
			struct type* expr_type = expr_typecheck(s->expr);
			result = expr_type->errorless && result;
			type_delete(expr_type);
			break;
		case STMT_IF_ELSE:
			;
			struct type* if_cond_type = expr_typecheck(s->expr);
			if(if_cond_type->kind != TYPE_BOOLEAN) {
				printf("type error: must use a boolean in an condition for an if statement, not a ");
				type_print(if_cond_type);
				printf("\n");
				result = 0;
			}
			result = if_cond_type->errorless && result;
			result = stmt_typecheck(s->body, return_type) && result;

			// If the if statement has an else block, resolve that too
			if(s->else_body) {
				result = stmt_typecheck(s->else_body, return_type) && result;
			}
			type_delete(if_cond_type);
			break;
		case STMT_BLOCK:
			result = stmt_typecheck(s->body, return_type) && result;
			break;
		case STMT_FOR:
			;
			struct type* init_type = expr_typecheck(s->init_expr);
			result = ((init_type) ? init_type->errorless : 1) && result;
			type_delete(init_type);

			// Ensure  middle expr in for loop is of type boolean
			struct type* for_cond_type = expr_typecheck(s->expr);
			if(for_cond_type && for_cond_type->kind != TYPE_BOOLEAN) {
				printf("type error: must use a boolean in middle expression of for loop, not a ");
				type_print(for_cond_type);
				printf("\n");
				result = 0;
			}
			result = ((for_cond_type) ? for_cond_type->errorless : 1) && result;
			type_delete(for_cond_type);

			struct type* next_type = expr_typecheck(s->next_expr);
			result = ((next_type) ? next_type->errorless : 1) && result;
			type_delete(next_type);

			result = stmt_typecheck(s->body, return_type) && result;
			break;
		case STMT_PRINT:
			;
			struct expr* curr = s->expr;
			while(curr) {
				struct type* t = expr_typecheck(curr);
				if(t->kind == TYPE_ARRAY || t->kind == TYPE_FUNCTION || t->kind == TYPE_VOID) {
					printf("type error: cannot print expression of type ");
					type_print(t);
					printf(" (");
					expr_print_single(curr);
					printf("). Only boolean, integer, character, and string are allowed.\n");
					result = 0;
				}
				result = t->errorless && result;
				curr = curr->next;
				type_delete(t);
			}
			break;
		case STMT_RETURN:
			;
			struct type* t = expr_typecheck(s->expr);
			if(!type_equals(t, return_type) && !(return_type->kind == TYPE_VOID && !t)) {
				printf("type error: cannot return expression (");
				expr_print(s->expr, 0);
				printf(") of type ");
				type_print(t);
				printf(" from function with return type ");
				type_print(return_type);
				printf("\n");
				result = 0;
			}
			result = ((t) ? t->errorless : 1) && result;
			type_delete(t);
			break;
	}

	return stmt_typecheck(s->next, return_type) && result;



}

void printTabsStmt(int tabLevel) {

	int i;
	for(i = 0; i < tabLevel; i++) {
		printf("\t");
	}

}

void stmt_codegen_globals(struct stmt* s, FILE* fp) {

	if (!s) {
		return;
	}


	switch(s->kind) {
		case STMT_DECL:
			decl_codegen_globals(s->decl, fp);
			break;
		case STMT_IF_ELSE:
			expr_codegen_globals(s->expr, fp);
			stmt_codegen_globals(s->body, fp);
			stmt_codegen_globals(s->else_body, fp);
			break;
		case STMT_BLOCK:
			stmt_codegen_globals(s->body, fp);
			break;
		case STMT_FOR:
			expr_codegen_globals(s->init_expr, fp);
			expr_codegen_globals(s->expr, fp);
			expr_codegen_globals(s->next_expr, fp);
			stmt_codegen_globals(s->body, fp);
			break;
		case STMT_PRINT:
		case STMT_RETURN:
		case STMT_EXPR:
			expr_codegen_globals(s->expr, fp);
			break;
	}

	stmt_codegen_globals(s->next, fp);

}

void stmt_codegen(struct stmt* s, FILE* fp, const char* enclosing_func_name) {

	if (!s) {
		return;
	}


	switch(s->kind) {
		case STMT_DECL:
			decl_codegen(s->decl, fp);
			break;
		case STMT_IF_ELSE:
			expr_codegen(s->expr, fp);
			int if_label1 = label_create();
			const char* if_label1_name = label_name(if_label1);
			int if_label2;
			const char* if_label2_name;
			fprintf(fp, "CMP $1, %s\n", scratch_name(s->expr->register_number));
			fprintf(fp, "JNE %s\n", if_label1_name);
			stmt_codegen(s->body, fp, enclosing_func_name);
			
			// if there's an else block, we need to create a new label and jump to it
			if (s->else_body) {
				if_label2 = label_create();
				if_label2_name = label_name(if_label2);
				fprintf(fp, "JMP %s\n", if_label2_name);
			}
			
			// Print first label after if block
			fprintf(fp, "%s:\n", if_label1_name);

			// If there's an else block, codegen the else block and then place the second label after it
			if(s->else_body) {
				stmt_codegen(s->else_body, fp, enclosing_func_name);
				fprintf(fp, "%s:\n", if_label2_name);
				free((char*) if_label2_name);
			}

			// Free the expression register
			scratch_free(s->expr->register_number);
			free((char*) if_label1_name);
			break;
		case STMT_BLOCK:
			stmt_codegen(s->body, fp, enclosing_func_name);
			break;
		case STMT_FOR:
			;
			int for_label1 = label_create();
			const char* for_label1_name = label_name(for_label1);
			int for_label2 = label_create();
			const char* for_label2_name = label_name(for_label2);
			expr_codegen(s->init_expr, fp);
			fprintf(fp, "%s:\n", for_label1_name);
			expr_codegen(s->expr, fp);
			if(s->expr) {
				fprintf(fp, "CMP $1, %s\n", scratch_name(s->expr->register_number));
				fprintf(fp, "JNE %s\n", for_label2_name);
			}
			stmt_codegen(s->body, fp, enclosing_func_name);
			expr_codegen(s->next_expr, fp);
			fprintf(fp, "JMP %s\n", for_label1_name);
			fprintf(fp, "%s:\n", for_label2_name);

			// Free scratch registers
			if (s->init_expr) scratch_free(s->init_expr->register_number);
			if (s->expr) scratch_free(s->expr->register_number);
			if (s->next_expr) scratch_free(s->next_expr->register_number);

			// Free label names
			free((char*) for_label1_name);
			free((char*) for_label2_name);
			break;
		case STMT_PRINT:
			;
			// Go down the epression list
			struct expr* curr = s->expr;
			while (curr) {
				// Generate code for the expression
				expr_codegen(curr, fp);

				// Move the expression register into a function argument register
				param_list_move_register_to_function_argument_register(curr->register_number, 0, fp);

				// Typecheck the expression
				struct type* t = expr_typecheck(curr);

				int register_num;

				// Call a different function based on the type
				switch(t->kind) {
					case TYPE_BOOLEAN:
						register_num = expr_call_function_codegen("print_boolean", fp);
						break;
					case TYPE_CHARACTER:
						register_num = expr_call_function_codegen("print_character", fp);
						break;
					case TYPE_INTEGER:
						register_num = expr_call_function_codegen("print_integer", fp);
						break;
					case TYPE_STRING:
						register_num = expr_call_function_codegen("print_string", fp);
						break;
					default:
						break;
				}

				// Free the expression register
				scratch_free(curr->register_number);

				// Free the return value register
				scratch_free(register_num);

				// Move to next expression
				curr = curr->next;
			}
			break;
		case STMT_RETURN:
			expr_codegen(s->expr, fp);

			if(s->expr) {
				fprintf(fp, "MOV %s, %%rax\n", scratch_name(s->expr->register_number));
				scratch_free(s->expr->register_number);
			}			
			fprintf(fp, "JMP %s_epilogue\n", enclosing_func_name);
			break;
		case STMT_EXPR:
			expr_codegen(s->expr, fp);
			scratch_free(s->expr->register_number);
			break;
	}

	stmt_codegen(s->next, fp, enclosing_func_name);

}
