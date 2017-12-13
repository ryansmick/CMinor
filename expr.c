#include "expr.h"
#include "symbol.h"
#include "scope.h"
#include "utils.h"
#include "param_list.h"
#include "scratch.h"
#include "label.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Create and return expr struct
struct expr* expr_create(expr_t kind, int precedence, struct expr* left, struct expr* right, const char* name, int literal_value, const char* string_literal, const char* original_literal_value) {
	struct expr* e = malloc(sizeof(*e));

	// Add values to e
	e->kind = kind;
	e->precedence = precedence;
	e->left = left;
	e->right = right;
	e->name = name;
	e->literal_value = literal_value;
	e->string_literal = string_literal;
	e->original_literal_value = original_literal_value;

	return e;
}

void expr_print(struct expr* e, int parentPrecedence) {

	if(!e) {
		return;
	}

	int printParens = 0;
	if(e->precedence < parentPrecedence) {
		printParens = 1;
		printf("(");
	}

	expr_print_single(e);	

	if(printParens) {
		printf(")");
	}

	if(e->next) {
		printf(", ");
		expr_print(e->next, 0);
	}

}

void expr_print_single(struct expr* e) {

	switch(e->kind) {
		case EXPR_ASSIGN:
		case EXPR_OR:
		case EXPR_AND:
		case EXPR_LT:
		case EXPR_LE:
		case EXPR_GT:
		case EXPR_GE:
		case EXPR_EQUAL:
		case EXPR_NE:
		case EXPR_PLUS:
		case EXPR_MINUS:
		case EXPR_MULT:
		case EXPR_DIVIDE:
		case EXPR_MODULUS:
		case EXPR_XOR:
			expr_print(e->left, e->precedence);
			expr_print_operator(e->kind);
			expr_print(e->right, e->precedence);
			break;
		case EXPR_UNARY_MINUS:
		case EXPR_NOT:
			expr_print_operator(e->kind);
			expr_print(e->right, e->precedence);
			break;
		case EXPR_INCREMENT:
		case EXPR_DECREMENT:
			expr_print(e->left, e->precedence);
			expr_print_operator(e->kind);
			break;
		case EXPR_SUBSCRIPT:
			expr_print(e->left, e->precedence);
			printf("[");
			expr_print(e->right, 0);
			printf("]");
			break;
		case EXPR_ARRAY_INITIALIZER:
			printf("{");
			expr_print(e->right, e->precedence);
			printf("}");
			break;
		case EXPR_NAME:
			printf("%s", e->name);
			break;
		case EXPR_CALL:
			expr_print(e->left, 0);
			printf("(");
			expr_print(e->right, 0);
			printf(")");
			break;
		case EXPR_INTEGER_LITERAL:
			printf("%d", e->literal_value);
			break;
		case EXPR_STRING_LITERAL:
			printf("%s", e->original_literal_value);
			break;
		case EXPR_CHAR_LITERAL:
			printf("%s", e->original_literal_value);
			break;
		case EXPR_TRUE:
			printf("true");
			break;
		case EXPR_FALSE:
			printf("false");
			break;
	}

}

void expr_print_operator(expr_t e) {

	switch(e) {

		case EXPR_ASSIGN:
			printf("=");
			break;
		case EXPR_OR:
			printf("||");
			break;
		case EXPR_AND:
			printf("&&");
			break;
		case EXPR_LT:
			printf("<");
			break;
		case EXPR_LE:
			printf("<=");
			break;
		case EXPR_GT:
			printf(">");
			break;
		case EXPR_GE:
			printf(">=");
			break;
		case EXPR_EQUAL:
			printf("==");
			break;
		case EXPR_NE:
			printf("!=");
			break;
		case EXPR_PLUS:
			printf("+");
			break;
		case EXPR_MINUS:
		case EXPR_UNARY_MINUS:
			printf(" -");
			break;
		case EXPR_MULT:
			printf("*");
			break;
		case EXPR_DIVIDE:
			printf("/");
			break;
		case EXPR_MODULUS:
			printf("%%");
			break;
		case EXPR_XOR:
			printf("^");
			break;
		case EXPR_NOT:
			printf("!");
			break;
		case EXPR_INCREMENT:
			printf("++");
			break;
		case EXPR_DECREMENT:
			printf("--");
			break;
		case EXPR_TRUE:
		case EXPR_FALSE:
		case EXPR_SUBSCRIPT:
		case EXPR_NAME:
		case EXPR_CALL:
		case EXPR_INTEGER_LITERAL:
		case EXPR_CHAR_LITERAL:
		case EXPR_STRING_LITERAL:
		case EXPR_ARRAY_INITIALIZER:
			break;
	}

}

int expr_resolve(struct expr* e, int verbose) {

	if (!e) {
		return 1;
	}

	int result = 1;

	switch(e->kind) {
		case EXPR_ASSIGN:
		case EXPR_OR:
		case EXPR_AND:
		case EXPR_LT:
		case EXPR_LE:
		case EXPR_GT:
		case EXPR_GE:
		case EXPR_EQUAL:
		case EXPR_NE:
		case EXPR_PLUS:
		case EXPR_MINUS:
		case EXPR_MULT:
		case EXPR_DIVIDE:
		case EXPR_MODULUS:
		case EXPR_XOR:
		case EXPR_SUBSCRIPT:
		case EXPR_CALL:
			result = expr_resolve(e->left, verbose);
			result = expr_resolve(e->right, verbose) && result;
			break;
		case EXPR_UNARY_MINUS:
		case EXPR_NOT:
		case EXPR_ARRAY_INITIALIZER:
			result =  expr_resolve(e->right, verbose);
			break;
		case EXPR_INCREMENT:
		case EXPR_DECREMENT:
			result =  expr_resolve(e->left, verbose);
			break;
		case EXPR_NAME:
			if(!scope_lookup(e->name)) {
				printf("resolve error: \"%s\" is not defined\n", e->name);
				result =  0;
			}
			else {
				struct symbol* s = scope_lookup(e->name);
				if (verbose) {
					printf("\"%s\" resolves to ", e->name);
					if (s->kind == SYMBOL_GLOBAL) {
						printf("global %s\n", s->name);
					}
					else if (s->kind == SYMBOL_LOCAL) {
						printf("local %d\n", s->which);
					}
					else if (s->kind == SYMBOL_PARAM) {
						printf("param %d\n", s->which);
					}
				}
				e->symbol = s;
				result = 1;
			}
			break;
		case EXPR_INTEGER_LITERAL:
		case EXPR_STRING_LITERAL:
		case EXPR_CHAR_LITERAL:
		case EXPR_TRUE:
		case EXPR_FALSE:
			result = 1;
			break;
	}

	return expr_resolve(e->next, verbose) && result;
}

struct expr* expr_copy(struct expr* e) {

	if (!e) {
		return 0;
	}

	struct expr* new_e = malloc(sizeof(*new_e));

	new_e->kind = e->kind;
	new_e->precedence = e->precedence;
	new_e->left = expr_copy(e->left);
	new_e->right = expr_copy(e->right);
	new_e->name = (e->name) ? strdup(e->name) : 0;
	new_e->symbol = symbol_copy(e->symbol);
	new_e->literal_value = e->literal_value;
	new_e->string_literal = (e->string_literal) ? strdup(e->string_literal) : 0;
	new_e->original_literal_value = (e->original_literal_value) ? strdup(e->original_literal_value) : 0;
	new_e->next = expr_copy(e->next);

	return new_e;

}

void expr_delete(struct expr* e) {

	if(!e) {
		return;
	}

	expr_delete(e->left);
	expr_delete(e->right);
	free((char*) e->name);
	symbol_delete(e->symbol);
	free((char*) e->string_literal);
	free((char*) e->original_literal_value);
	expr_delete(e->next);

	free(e);
}

struct type* expr_typecheck(struct expr* e) {

	if (!e) {
		return 0;
	}

	struct type* lt = expr_typecheck(e->left);
	struct type* rt = expr_typecheck(e->right);

	struct type* result;
	int errorless_value = 1;

	if(lt) {
		errorless_value = errorless_value && lt->errorless;
	}
	if(rt) {
		errorless_value = errorless_value && rt->errorless;
	}

	switch(e->kind) {
		case EXPR_ASSIGN:
			if(!type_equals(lt, rt)) {
				printf("type error: cannot assign expression of type ");
				type_print(rt);
				printf(" (");
				expr_print(e->right, 0);
				printf(") to variable of type ");
				type_print(lt);
				printf(" (");
				expr_print(e->left, 0);
				printf(")\n");
				errorless_value = 0;
			}
			result = type_copy(rt);
			break;
		case EXPR_OR:
		case EXPR_AND:
			if(!lt->kind == TYPE_BOOLEAN || !rt->kind == TYPE_BOOLEAN) {
				printf("type error: cannot perform ");
				expr_print_operator(e->kind);
				printf(" operation on types ");
				type_print(lt);
				printf(" (");
				expr_print(e->left, 0);
				printf(") and ");
				type_print(rt);
				printf(" (");
				expr_print(e->right, 0);
				printf(")\n");
				errorless_value = 0;
			}
			result = type_create(TYPE_BOOLEAN, 0, 0, 0);
			break;
		case EXPR_LT:
		case EXPR_LE:
		case EXPR_GT:
		case EXPR_GE:
			if(!lt->kind == TYPE_INTEGER || !rt->kind == TYPE_INTEGER) {
				printf("type error: cannot perform ");
				expr_print_operator(e->kind);
				printf(" operation on types ");
				type_print(lt);
				printf(" (");
				expr_print(e->left, 0);
				printf(") and ");
				type_print(rt);
				printf(" (");
				expr_print(e->right, 0);
				printf(")\n");
				errorless_value = 0;
			}
			result = type_create(TYPE_BOOLEAN, 0, 0, 0);;
			break;
		case EXPR_PLUS:
		case EXPR_MINUS:
		case EXPR_MULT:
		case EXPR_DIVIDE:
		case EXPR_MODULUS:
		case EXPR_XOR:
			if(!lt->kind == TYPE_INTEGER || !rt->kind == TYPE_INTEGER) {
				printf("type error: cannot perform ");
				expr_print_operator(e->kind);
				printf(" operation on types ");
				type_print(lt);
				printf(" (");
				expr_print(e->left, 0);
				printf(") and ");
				type_print(rt);
				printf(" (");
				expr_print(e->right, 0);
				printf(")\n");
				errorless_value = 0;
			}
			result = type_create(TYPE_INTEGER, 0, 0, 0);;
			break;
		case EXPR_EQUAL:
		case EXPR_NE:
			if(!type_equals(lt, rt)) {
				printf("type error: cannot perform ");
				expr_print_operator(e->kind);
				printf(" operation on mismatching types ");
				type_print(lt);
				printf(" (");
				expr_print(e->left, 0);
				printf(") and ");
				type_print(rt);
				printf(" (");
				expr_print(e->right, 0);
				printf(")\n");
				errorless_value = 0;
			}

			if(!lt->kind == TYPE_INTEGER && !lt->kind == TYPE_CHARACTER && !lt->kind == TYPE_STRING && !lt->kind == TYPE_BOOLEAN) {
				printf("type error: cannot perform ");
				expr_print_operator(e->kind);
				printf(" operation on expression of type ");
				type_print(lt);
				printf(" (");
				expr_print(e->left, 0);
				printf(")\n");
				errorless_value = 0;
			}

			if(!rt->kind == TYPE_INTEGER && !rt->kind == TYPE_CHARACTER && !rt->kind == TYPE_STRING && !rt->kind == TYPE_BOOLEAN) {
				printf("type error: cannot perform ");
				expr_print_operator(e->kind);
				printf(" operation on expression of type ");
				type_print(rt);
				printf(" (");
				expr_print(e->right, 0);
				printf(")\n");
				errorless_value = 0;
			}

			result = type_create(TYPE_BOOLEAN, 0, 0, 0);;
			break;
		case EXPR_UNARY_MINUS:
			if(rt->kind != TYPE_INTEGER) {
				printf("type error: cannot take the opposite of an expression of type ");
				type_print(rt);
				printf(" (");
				expr_print(e->right, 0);
				printf(")\n");
				errorless_value = 0;
			}

			result = type_create(TYPE_INTEGER, 0, 0, 0);
			break;
		case EXPR_NOT:
			if(rt->kind != TYPE_BOOLEAN) {
				printf("type error: cannot negate an expression of type ");
				type_print(rt);
				printf(" (");
				expr_print(e->right, 0);
				printf(")\n");
				errorless_value = 0;
			}

			result = type_create(TYPE_BOOLEAN, 0, 0, 0);
			break;
		case EXPR_INCREMENT:
		case EXPR_DECREMENT:
			if(!lt->kind == TYPE_INTEGER) {
				printf("type error: cannot perform ");
				expr_print_operator(e->kind);
				printf(" operation on expression of type ");
				type_print(lt);
				printf(" (");
				expr_print(e->left, 0);
				printf(")\n");
				errorless_value = 0;
			}

			result = type_create(TYPE_INTEGER, 0, 0, 0);
			break;
		case EXPR_SUBSCRIPT:
			;
			int local_error = 0;
			if (lt->kind != TYPE_ARRAY) {
				printf("type error: cannot subscript expression of type ");
				type_print(lt);
				printf(" (");
				expr_print(e->left, 0);
				printf(")\n");
				errorless_value = 0;
				local_error += 1;
			}
			
			if (rt->kind != TYPE_INTEGER) {
				printf("type error: subscript value must be an integer literal rather than a ");
				type_print(rt);
				printf(" (");
				expr_print(e->right, 0);
				printf(")\n");
				errorless_value = 0;
				local_error += 2;
			}

			if(local_error == 0 || local_error == 2) {
				result = type_copy(lt->subtype);
			}
			else {
				result = type_copy(lt);
			}
			break;
		case EXPR_ARRAY_INITIALIZER:
			;
			struct expr* curr = e->right;
			int counter = 0;
			while(curr) {
				struct type* curr_type = expr_typecheck(curr);
				if (!type_equals(rt, curr_type)) {
					printf("type error: element in array initializer of type ");
					type_print(curr_type);
					printf(" (");
					expr_print_single(curr);
					printf(") does not match type of first element, which is ");
					type_print(rt);
					printf(" (");
					expr_print_single(e->right);
					printf(")\n");
					errorless_value = 0;
				}
				type_delete(curr_type);
				counter += 1;
				curr = curr->next;
			}
			result = type_create(TYPE_ARRAY, type_copy(rt), expr_create(EXPR_INTEGER_LITERAL, 10, 0, 0, 0, counter, 0, 0), 0);
			break;
		case EXPR_NAME:
			result = type_copy(e->symbol->type);
			break;
		case EXPR_CALL:
			if(lt->kind != TYPE_FUNCTION) {
				printf("type error: cannot call expression of type ");
				type_print(lt);
				printf(" (");
				expr_print(e->left, 0);
				printf(")\n");
				errorless_value = 0;
			}
			
			// Check types of every parameter
			errorless_value = param_list_check_types(lt->params, e->right) && errorless_value;

			if (lt->kind == TYPE_FUNCTION) {
				result = type_copy(lt->subtype);
			}
			else {
				result = type_copy(lt);
			}
			break;
		case EXPR_INTEGER_LITERAL:
			result = type_create(TYPE_INTEGER, 0, 0, 0);
			break;
		case EXPR_STRING_LITERAL:
			result = type_create(TYPE_STRING, 0, 0, 0);
			break;
		case EXPR_CHAR_LITERAL:
			result = type_create(TYPE_CHARACTER, 0, 0, 0);
			break;
		case EXPR_TRUE:
		case EXPR_FALSE:
			result = type_create(TYPE_BOOLEAN, 0, 0, 0);
			break;
	}

	result->errorless = errorless_value;
	
	type_delete(lt);
	type_delete(rt);

	return result;

}


int expr_list_all_constants(struct type* t, struct expr* e) {

	if (!e || !t) {
		return 1;
	}

	int result = expr_list_all_constants(t, e->next);
	int currentResult = 1;

	if ((t->kind == TYPE_INTEGER && e->kind != EXPR_INTEGER_LITERAL) || (t->kind == TYPE_CHARACTER && e->kind != EXPR_CHAR_LITERAL) || (t->kind == TYPE_STRING && e->kind != EXPR_STRING_LITERAL) || (t->kind == TYPE_BOOLEAN && !(e->kind == EXPR_TRUE || e->kind == EXPR_FALSE)) || t->kind == TYPE_ARRAY || t->kind == TYPE_FUNCTION || t->kind == TYPE_VOID) {
		currentResult = 0;
	}

	if (currentResult == 0) {
		printf("type error: element ");
		expr_print_single(e);
		printf(" must be a literal of type ");
		type_print(t);
		printf("\n");
	}
	
	return result && currentResult;
}

// Function to return the literal value of an expression
const char* expr_get_literal_value(struct expr* e) {

	if(!e) {
		return "";
	}

	char* literal_value;
	int bufsize;
	if(e->kind == EXPR_INTEGER_LITERAL) {
		bufsize = digits_in_integer(e->literal_value) + 1;
		literal_value = malloc(sizeof(char) * bufsize);
		snprintf(literal_value, bufsize, "%d", e->literal_value);		
	}
	else if(e->kind == EXPR_CHAR_LITERAL) {
		bufsize = digits_in_integer(e->literal_value) + 1;;
		literal_value = malloc(sizeof(char) * bufsize);
		snprintf(literal_value, bufsize, "%d", e->literal_value);
	}
	else if(e->kind == EXPR_STRING_LITERAL) {
		bufsize = strlen(e->string_literal) + 3;
		literal_value = malloc(sizeof(char) * bufsize);
		snprintf(literal_value, bufsize, "\"%s\"", e->string_literal);
	}
	else if(e->kind == EXPR_TRUE || e->kind == EXPR_FALSE) {
		bufsize = 2;
		literal_value = malloc(sizeof(char) * bufsize);
		snprintf(literal_value, bufsize, "%d", e->kind == EXPR_TRUE ? 1 : 0);
	}
	else if(e->kind == EXPR_ARRAY_INITIALIZER) {
		literal_value = (char*) expr_get_literal_value(e->right);
		bufsize = strlen(literal_value);
	}

	// Handle an expr list
	if (e->next) {
		const char* right = expr_get_literal_value(e->next);
		bufsize = bufsize + strlen(right) + 3;
		char* new_literal_value = malloc(sizeof(char) * bufsize);
		snprintf(new_literal_value, bufsize, "%s, %s", literal_value, right);
		free(literal_value);
		free((char*) right);
		literal_value = new_literal_value;
	}

	return literal_value;
}

void expr_codegen_globals(struct expr* e, FILE* fp) {

		
	if (!e) {
		return;
	}

	switch(e->kind) {
		case EXPR_STRING_LITERAL:
			e->global_name = expr_generate_string_global_name();

			// Remove quotes from original_literal_value
			int length = strlen(e->original_literal_value);
			char* newbuf = malloc(sizeof(char) * (length + 1));
			memcpy(newbuf, e->original_literal_value + 1, length-2);
			newbuf[length-2] = '\0';
			
			// Print the new value
			fprintf(fp, "%s: .string \"%s\"\n", e->global_name, newbuf);
			free(newbuf);
			break;
		case EXPR_CHAR_LITERAL:
		case EXPR_ASSIGN:
		case EXPR_OR:
		case EXPR_AND:
		case EXPR_LT:
		case EXPR_LE:
		case EXPR_GT:
		case EXPR_GE:
		case EXPR_PLUS:
		case EXPR_MINUS:
		case EXPR_MULT:
		case EXPR_DIVIDE:
		case EXPR_MODULUS:
		case EXPR_XOR:
		case EXPR_EQUAL:
		case EXPR_NE:
		case EXPR_UNARY_MINUS:
		case EXPR_NOT:
		case EXPR_INCREMENT:
		case EXPR_DECREMENT:
		case EXPR_SUBSCRIPT:
		case EXPR_ARRAY_INITIALIZER:
		case EXPR_NAME:
		case EXPR_CALL:
		case EXPR_INTEGER_LITERAL:
		case EXPR_TRUE:
		case EXPR_FALSE:
			expr_codegen_globals(e->left, fp);
			expr_codegen_globals(e->right, fp);
			break;
	}

	// Generate global code for next global in expr list
	expr_codegen_globals(e->next, fp);

}

const char* expr_generate_string_global_name() {

	static int number = 1;

	int bufsize = digits_in_integer(number) + 5;
	char* name = malloc(sizeof(char) * bufsize);
	snprintf(name, bufsize, ".str%d", number);

	number++;

	return name;

}

char* translate_expr_t_to_string(expr_t num) {

	switch(num) {
		case EXPR_STRING_LITERAL:
			return "EXPR_STRING_LITERAL";
			break;
		case EXPR_CHAR_LITERAL:
			return "EXPR_CHAR_LITERAL";
			break;
		case EXPR_ASSIGN:
			return "EXPR_ASSIGN";
			break;
		case EXPR_OR:
			return "EXPR_OR";
			break;
		case EXPR_AND:
			return "EXPR_AND";
			break;
		case EXPR_LT:
			return "EXPR_LT";
			break;
		case EXPR_LE:
			return "EXPR_LE";
			break;
		case EXPR_GT:
			return "EXPR_GT";
			break;
		case EXPR_GE:
			return "EXPR_GE";
			break;
		case EXPR_PLUS:
			return "EXPR_PLUS";
			break;
		case EXPR_MINUS:
			return "EXPR_MINUS";
			break;
		case EXPR_MULT:
			return "EXPR_MULT";
			break;
		case EXPR_DIVIDE:
			return "EXPR_DIVIDE";
			break;
		case EXPR_MODULUS:
			return "EXPR_MODULUS";
			break;
		case EXPR_XOR:
			return "EXPR_XOR";
			break;
		case EXPR_EQUAL:
			return "EXPR_EQUAL";
			break;
		case EXPR_NE:
			return "EXPR_NE";
			break;
		case EXPR_UNARY_MINUS:
			return "EXPR_UNARY_MINUS";
			break;
		case EXPR_NOT:
			return "EXPR_NOT";
			break;
		case EXPR_INCREMENT:
			return "EXPR_INCREMENT";
			break;
		case EXPR_DECREMENT:
			return "EXPR_DECREMENT";
			break;
		case EXPR_SUBSCRIPT:
			return "EXPR_SUBSCRIPT";
			break;
		case EXPR_ARRAY_INITIALIZER:
			return "EXPR_ARRAY_INITIALIZER";
			break;
		case EXPR_NAME:
			return "EXPR_NAME";
			break;
		case EXPR_CALL:
			return "EXPR_CALL";
			break;
		case EXPR_INTEGER_LITERAL:
			return "EXPR_INTEGER_LITERAL";
			break;
		case EXPR_TRUE:
			return "EXPR_TRUE";
			break;
		case EXPR_FALSE:
			return "EXPR_FALSE";
			break;
		default:
			return "Could not identify value";
			break;
	}


}

void expr_codegen(struct expr* e, FILE* fp) {

	if(!e) {
		return;
	}

	switch(e->kind) {
		case EXPR_STRING_LITERAL:
			e->register_number = scratch_alloc();
			fprintf(fp, "MOVQ $%s, %s\n", e->global_name, scratch_name(e->register_number));
			break;
		case EXPR_CHAR_LITERAL:
			e->register_number = scratch_alloc();
			fprintf(fp, "MOVQ $%d, %s\n", e->literal_value, scratch_name(e->register_number));
			break;
		case EXPR_ASSIGN:
			expr_codegen(e->right, fp);
			if(e->left->kind == EXPR_SUBSCRIPT) {
				expr_codegen(e->left->left, fp);
				expr_codegen(e->left->right, fp);
				fprintf(fp, "MOVQ %s, 0(%s,%s,8)\n", scratch_name(e->right->register_number), scratch_name(e->left->left->register_number), scratch_name(e->left->right->register_number));
				scratch_free(e->left->left->register_number);
				scratch_free(e->left->right->register_number);
			}
			else {
				const char* left_symbol = symbol_codegen(e->left->symbol);
				fprintf(fp, "MOVQ %s, %s\n", scratch_name(e->right->register_number), left_symbol);
				free((char*) left_symbol);
			}
			e->register_number = e->right->register_number;
			break;
		case EXPR_OR:
			expr_codegen(e->left, fp);
			expr_codegen(e->right, fp);
			fprintf(fp, "OR %s, %s\n", scratch_name(e->left->register_number), scratch_name(e->right->register_number));
			fprintf(fp, "AND $1, %s\n", scratch_name(e->right->register_number));
			scratch_free(e->left->register_number);
			e->register_number = e->right->register_number;
			break;
		case EXPR_AND:
			expr_codegen(e->left, fp);
			expr_codegen(e->right, fp);
			fprintf(fp, "AND %s, %s\n", scratch_name(e->left->register_number), scratch_name(e->right->register_number));
			fprintf(fp, "AND $1, %s\n", scratch_name(e->right->register_number));
			scratch_free(e->left->register_number);
			e->register_number = e->right->register_number;	
			break;
		case EXPR_LT:
			expr_codegen(e->left, fp);
			expr_codegen(e->right, fp);
			int lt_label1 = label_create();
			int lt_label2 = label_create();
			fprintf(fp, "CMP %s, %s\n", scratch_name(e->right->register_number), scratch_name(e->left->register_number));
			fprintf(fp, "JL %s\n", label_name(lt_label1));
			fprintf(fp, "MOVQ $0, %s\n", scratch_name(e->right->register_number));
			fprintf(fp, "JMP %s\n", label_name(lt_label2));
			fprintf(fp, "%s:\n", label_name(lt_label1));
			fprintf(fp, "MOVQ $1, %s\n", scratch_name(e->right->register_number));
			fprintf(fp, "%s:\n", label_name(lt_label2));
			e->register_number = e->right->register_number;
			scratch_free(e->left->register_number);
			break;
		case EXPR_LE:
			expr_codegen(e->left, fp);
			expr_codegen(e->right, fp);
			int le_label1 = label_create();
			int le_label2 = label_create();
			fprintf(fp, "CMP %s, %s\n", scratch_name(e->right->register_number), scratch_name(e->left->register_number));
			fprintf(fp, "JLE %s\n", label_name(le_label1));
			fprintf(fp, "MOVQ $0, %s\n", scratch_name(e->right->register_number));
			fprintf(fp, "JMP %s\n", label_name(le_label2));
			fprintf(fp, "%s:\n", label_name(le_label1));
			fprintf(fp, "MOVQ $1, %s\n", scratch_name(e->right->register_number));
			fprintf(fp, "%s:\n", label_name(le_label2));
			e->register_number = e->right->register_number;
			scratch_free(e->left->register_number);
			break;
		case EXPR_GT:
			expr_codegen(e->left, fp);
			expr_codegen(e->right, fp);
			int gt_label1 = label_create();
			int gt_label2 = label_create();
			fprintf(fp, "CMP %s, %s\n", scratch_name(e->right->register_number), scratch_name(e->left->register_number));
			fprintf(fp, "JG %s\n", label_name(gt_label1));
			fprintf(fp, "MOVQ $0, %s\n", scratch_name(e->right->register_number));
			fprintf(fp, "JMP %s\n", label_name(gt_label2));
			fprintf(fp, "%s:\n", label_name(gt_label1));
			fprintf(fp, "MOVQ $1, %s\n", scratch_name(e->right->register_number));
			fprintf(fp, "%s:\n", label_name(gt_label2));
			e->register_number = e->right->register_number;
			scratch_free(e->left->register_number);
			break;
		case EXPR_GE:
			expr_codegen(e->left, fp);
			expr_codegen(e->right, fp);
			int ge_label1 = label_create();
			int ge_label2 = label_create();
			fprintf(fp, "CMP %s, %s\n", scratch_name(e->right->register_number), scratch_name(e->left->register_number));
			fprintf(fp, "JGE %s\n", label_name(ge_label1));
			fprintf(fp, "MOVQ $0, %s\n", scratch_name(e->right->register_number));
			fprintf(fp, "JMP %s\n", label_name(ge_label2));
			fprintf(fp, "%s:\n", label_name(ge_label1));
			fprintf(fp, "MOVQ $1, %s\n", scratch_name(e->right->register_number));
			fprintf(fp, "%s:\n", label_name(ge_label2));
			e->register_number = e->right->register_number;
			scratch_free(e->left->register_number);
			break;
		case EXPR_PLUS:
			expr_codegen(e->left, fp);
			expr_codegen(e->right, fp);
			fprintf(fp, "ADDQ %s, %s\n", scratch_name(e->left->register_number), scratch_name(e->right->register_number));
			e->register_number = e->right->register_number;
			scratch_free(e->left->register_number);
			break;
		case EXPR_MINUS:
			expr_codegen(e->left, fp);
			expr_codegen(e->right, fp);
			fprintf(fp, "SUBQ %s, %s\n", scratch_name(e->right->register_number), scratch_name(e->left->register_number));
			e->register_number = e->left->register_number;
			scratch_free(e->right->register_number);	
			break;
		case EXPR_MULT:
			expr_codegen(e->left, fp);
			expr_codegen(e->right, fp);
			fprintf(fp, "MOVQ %s, %%rax\n", scratch_name(e->left->register_number));
			fprintf(fp, "IMULQ %s\n", scratch_name(e->right->register_number));
			fprintf(fp, "MOVQ %%rax, %s\n", scratch_name(e->right->register_number));
			e->register_number = e->right->register_number;
			scratch_free(e->left->register_number);
			break;
		case EXPR_DIVIDE:
			expr_codegen(e->left, fp);
			expr_codegen(e->right, fp);
			fprintf(fp, "MOVQ %s, %%rax\n", scratch_name(e->left->register_number));
			fprintf(fp, "CQO\n");
			fprintf(fp, "IDIVQ %s\n", scratch_name(e->right->register_number));
			fprintf(fp, "MOVQ %%rax, %s\n", scratch_name(e->right->register_number));
			scratch_free(e->left->register_number);
			e->register_number = e->right->register_number;
			break;
		case EXPR_MODULUS:
			expr_codegen(e->left, fp);
			expr_codegen(e->right, fp);
			fprintf(fp, "MOVQ %s, %%rax\n", scratch_name(e->left->register_number));
			fprintf(fp, "CDQ\n");
			fprintf(fp, "IDIVQ %s\n", scratch_name(e->right->register_number));
			fprintf(fp, "MOVQ %%rdx, %s\n", scratch_name(e->right->register_number));
			scratch_free(e->left->register_number);
			e->register_number = e->right->register_number;
			break;
		case EXPR_XOR:
			expr_codegen(e->left, fp);
			expr_codegen(e->right, fp);
			param_list_move_register_to_function_argument_register(e->left->register_number, 0, fp);
			param_list_move_register_to_function_argument_register(e->right->register_number, 1, fp);
			e->register_number = expr_call_function_codegen("integer_power", fp);
			scratch_free(e->left->register_number);
			scratch_free(e->right->register_number);
			break;
		case EXPR_EQUAL:
			expr_codegen(e->left, fp);
			expr_codegen(e->right, fp);
			struct type* t = expr_typecheck(e->right);

			if (t->kind == TYPE_STRING) {
				param_list_move_register_to_function_argument_register(e->left->register_number, 0, fp);
				param_list_move_register_to_function_argument_register(e->right->register_number, 1, fp);
				e->register_number = expr_call_function_codegen("string_equals", fp);
				scratch_free(e->left->register_number);
				scratch_free(e->left->register_number);
			}
			else {
				int eq_label1 = label_create();
				int eq_label2 = label_create();
				fprintf(fp, "CMP %s, %s\n", scratch_name(e->right->register_number), scratch_name(e->left->register_number));
				fprintf(fp, "JE %s\n", label_name(eq_label1));
				fprintf(fp, "MOVQ $0, %s\n", scratch_name(e->right->register_number));
				fprintf(fp, "JMP %s\n", label_name(eq_label2));
				fprintf(fp, "%s:\n", label_name(eq_label1));
				fprintf(fp, "MOVQ $1, %s\n", scratch_name(e->right->register_number));
				fprintf(fp, "%s:\n", label_name(eq_label2));
				e->register_number = e->right->register_number;
				scratch_free(e->left->register_number);
			}
			break;
		case EXPR_NE:
			expr_codegen(e->left, fp);
			expr_codegen(e->right, fp);
			struct type* ne_t = expr_typecheck(e->right);

			if (ne_t->kind == TYPE_STRING) {
				param_list_move_register_to_function_argument_register(e->left->register_number, 0, fp);
				param_list_move_register_to_function_argument_register(e->right->register_number, 1, fp);
				e->register_number = expr_call_function_codegen("string_equals", fp);
				scratch_free(e->left->register_number);
				scratch_free(e->left->register_number);
			}
			else {
				int eq_label1 = label_create();
				int eq_label2 = label_create();
				fprintf(fp, "CMP %s, %s\n", scratch_name(e->right->register_number), scratch_name(e->left->register_number));
				fprintf(fp, "JE %s\n", label_name(eq_label1));
				fprintf(fp, "MOVQ $0, %s\n", scratch_name(e->right->register_number));
				fprintf(fp, "JMP %s\n", label_name(eq_label2));
				fprintf(fp, "%s:\n", label_name(eq_label1));
				fprintf(fp, "MOVQ $1, %s\n", scratch_name(e->right->register_number));
				fprintf(fp, "%s:\n", label_name(eq_label2));
				e->register_number = e->right->register_number;
				scratch_free(e->left->register_number);
			}

			fprintf(fp, "NOT %s\n", scratch_name(e->register_number));
			fprintf(fp, "AND $1, %s\n", scratch_name(e->register_number));
			break;
		case EXPR_UNARY_MINUS:
			expr_codegen(e->right, fp);
			fprintf(fp, "MOVQ $-1, %%rax\n");
			fprintf(fp, "IMULQ %s\n", scratch_name(e->right->register_number));
			fprintf(fp, "MOVQ %%rax, %s\n", scratch_name(e->right->register_number));
			e->register_number = e->right->register_number;
			break;
		case EXPR_NOT:
			expr_codegen(e->right, fp);
			fprintf(fp, "NOT %s\n", scratch_name(e->right->register_number));
			fprintf(fp, "AND $1, %s\n", scratch_name(e->right->register_number));
			e->register_number = e->right->register_number;
			break;
		case EXPR_INCREMENT:
			expr_codegen(e->left, fp);
			int increment_reg = scratch_alloc();
			fprintf(fp, "MOVQ %s, %s\n", scratch_name(e->left->register_number), scratch_name(increment_reg));
			fprintf(fp, "ADDQ $1, %s\n", scratch_name(increment_reg));
			const char* left_symbol2 = symbol_codegen(e->left->symbol);
			fprintf(fp, "MOVQ %s, %s\n", scratch_name(increment_reg), left_symbol2);
			free((char*) left_symbol2);
			scratch_free(increment_reg);
			e->register_number = e->left->register_number;
			break;
		case EXPR_DECREMENT:
			expr_codegen(e->left, fp);
			int decrement_reg = scratch_alloc();
			fprintf(fp, "MOVQ %s, %s\n", scratch_name(e->left->register_number), scratch_name(decrement_reg));
			fprintf(fp, "SUBQ $1, %s\n", scratch_name(decrement_reg));
			const char* left_symbol3 = symbol_codegen(e->left->symbol);
			fprintf(fp, "MOVQ %s, %s\n", scratch_name(decrement_reg), left_symbol3);
			free((char*) left_symbol3);
			scratch_free(decrement_reg);
			e->register_number = e->left->register_number;	
			break;
		case EXPR_SUBSCRIPT:
			expr_codegen(e->left, fp);
			expr_codegen(e->right, fp);
			fprintf(fp, "MOVQ 0(%s,%s,8), %s\n", scratch_name(e->left->register_number), scratch_name(e->right->register_number), scratch_name(e->right->register_number));
			e->register_number = e->right->register_number;
			scratch_free(e->left->register_number);
			break;
		case EXPR_ARRAY_INITIALIZER:
			printf("codegen error: local arrays not supported\n");
			exit(1);
			break;
		case EXPR_NAME:
			e->register_number = scratch_alloc();
			const char* var_loc = symbol_codegen(e->symbol);
			const char* register_name = scratch_name(e->register_number);
			fprintf(fp, "MOVQ %s, %s\n", var_loc, register_name);
			free((char*) var_loc);
			break;
		case EXPR_CALL:
			// Generate argument list code
			param_list_expr_argument_list_codegen(e->right, fp);

			// Call the function and store the return value in a new register
			e->register_number = expr_call_function_codegen(e->left->name, fp);
			break;
		case EXPR_INTEGER_LITERAL:
			;
			int reg = scratch_alloc();
			const char* reg_name = scratch_name(reg);
			fprintf(fp, "MOVQ $%d, %s\n", e->literal_value, reg_name);
			e->register_number = reg;
			break;
		case EXPR_TRUE:
			e->register_number = scratch_alloc();
			fprintf(fp, "MOVQ $%d, %s\n", 1, scratch_name(e->register_number));
			break;
		case EXPR_FALSE:
			e->register_number = scratch_alloc();
			fprintf(fp, "MOVQ $%d, %s\n", 0, scratch_name(e->register_number));
			break;
	}

}

int expr_call_function_codegen(const char* function_name, FILE* fp) {

	// Store caller saved registers
	fprintf(fp, "PUSHQ %%r10\n");
	fprintf(fp, "PUSHQ %%r11\n");

	// Call function
	fprintf(fp, "CALL %s\n", function_name);

	// Pop caller saved registers
	fprintf(fp, "POPQ %%r11\n");
	fprintf(fp, "POPQ %%r10\n");

	// Allocate new register for return value
	int register_number = scratch_alloc();

	// Move return value into newly created register
	fprintf(fp, "MOVQ %%rax, %s\n", scratch_name(register_number));

	// Return register
	return register_number;
}
