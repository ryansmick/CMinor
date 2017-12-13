// Author: Ryan Smick (rsmick)
// Main function for the cminor compiler

#include "parser.tab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "decl.h"
#include "scope.h"

extern FILE *yyin;
extern char* yytext;
extern int yylex();
extern int yyparse();
extern struct decl* parser_result;

void usage();

int main(int argc, char* argv[]) {

	// Print usage if command line arguments are incorrect
	if(argc != 3 && argc != 4) {
		usage();
	}
	
	if (argc == 3 && (strcmp(argv[1], "-scan") && strcmp(argv[1], "-print") && strcmp(argv[1], "-resolve") && strcmp(argv[1], "-typecheck"))) {
		usage();
	}

	if (argc == 4 && (strcmp(argv[1], "-codegen"))) {
		usage();
	}

	yyin = fopen(argv[2], "r");

	// Handle opening errors
	if(yyin == 0) {
		printf("Error: File %s could not be opened. Exiting...\n", argv[2]);
		return 1;
	}

	// Read through file and scan
	if(!strcmp(argv[1], "-scan")) {
		while(1) {
			int t = yylex();
			switch(t) {
				printf("%d\n", t);
				case TOKEN_ERROR:
					fprintf(stderr, "Scan error: \"%s\" is not a valid token\n", yytext);
					exit(1);
					break;
				case TOKEN_EOF:
					printf("TOKEN_EOF\n");
					exit(0);
					break;
				case TOKEN_ARRAY:
					printf("TOKEN_ARRAY\n");
					break;
				case TOKEN_BOOLEAN:
					printf("TOKEN_BOOLEAN\n");
					break;
				case TOKEN_CHAR:
					printf("TOKEN_CHAR\n");
					break;
				case TOKEN_ELSE:
					printf("TOKEN_ELSE\n");
					break;
				case TOKEN_FALSE:
					printf("TOKEN_FALSE\n");
					break;
				case TOKEN_FOR:
					printf("TOKEN_FOR\n");
					break;
				case TOKEN_FUNCTION:
					printf("TOKEN_FUNCTION\n");
					break;
				case TOKEN_IF:
					printf("TOKEN_IF\n");
					break;
				case TOKEN_INTEGER:
					printf("TOKEN_INTEGER\n");
					break;
				case TOKEN_PRINT:
					printf("TOKEN_PRINT\n");
					break;
				case TOKEN_RETURN:
					printf("TOKEN_RETURN\n");
					break;
				case TOKEN_STRING:
					printf("TOKEN_STRING\n");
					break;
				case TOKEN_TRUE:
					printf("TOKEN_TRUE\n");
					break;
				case TOKEN_VOID:
					printf("TOKEN_VOID\n");
					break;
				case TOKEN_WHILE:
					printf("TOKEN_WHILE\n");
					break;
				case TOKEN_NEWLINE:
					printf("TOKEN_NEWLINE\n");
					break;
				case TOKEN_NULL:
					printf("TOKEN_NULL\n");
					break;
				case TOKEN_LEFT_BRACKET:
					printf("TOKEN_LEFT_BRACKET\n");
					break;
				case TOKEN_RIGHT_BRACKET:
					printf("TOKEN_RIGHT_BRACKET\n");
					break;
				case TOKEN_LEFT_PAREN:
					printf("TOKEN_LEFT_PAREN\n");
					break;
				case TOKEN_RIGHT_PAREN:
					printf("TOKEN_RIGHT_PAREN\n");
					break;
				case TOKEN_LEFT_CURLY:
					printf("TOKEN_LEFT_CURLY\n");
					break;
				case TOKEN_RIGHT_CURLY:
					printf("TOKEN_RIGHT_CURLY\n");
					break;
				case TOKEN_PLUS:
					printf("TOKEN_PLUS\n");
					break;
				case TOKEN_MINUS:
					printf("TOKEN_MINUS\n");
					break;
				case TOKEN_NOT:
					printf("TOKEN_NOT\n");
					break;
				case TOKEN_XOR:
					printf("TOKEN_XOR\n");
					break;
				case TOKEN_MULT:
					printf("TOKEN_MULT\n");
					break;
				case TOKEN_DIVIDE:
					printf("TOKEN_DIVIDE\n");
					break;
				case TOKEN_MODULUS:
					printf("TOKEN_MODULUS\n");
					break;
				case TOKEN_LT:
					printf("TOKEN_LT\n");
					break;
				case TOKEN_GT:
					printf("TOKEN_GT\n");
					break;
				case TOKEN_ASSIGN:
					printf("TOKEN_ASSIGN\n");
					break;
				case TOKEN_AND:
					printf("TOKEN_AND\n");
					break;
				case TOKEN_OR:
					printf("TOKEN_OR\n");
					break;
				case TOKEN_COLON:
					printf("TOKEN_COLON\n");
					break;
				case TOKEN_SEMICOLON:
					printf("TOKEN_SEMICOLON\n");
					break;
				case TOKEN_COMMA:
					printf("TOKEN_COMMA\n");
					break;
				case TOKEN_INCREMENT:
					printf("TOKEN_INCREMENT\n");
					break;
				case TOKEN_DECREMENT:
					printf("TOKEN_DECREMENT\n");
					break;
				case TOKEN_EQUAL:
					printf("TOKEN_EQUAL\n");
					break;
				case TOKEN_GE:
					printf("TOKEN_GE\n");
					break;
				case TOKEN_LE:
					printf("TOKEN_LE\n");
					break;
				case TOKEN_NE:
					printf("TOKEN_NE\n");
					break;
				case TOKEN_ID:
					printf("TOKEN_ID %s\n", yytext);
					break;
				case TOKEN_INTEGER_LITERAL:
					printf("TOKEN_INTEGER_LITERAL %s\n", yytext);
					break;
				case TOKEN_STRING_LITERAL:
					printf("TOKEN_STRING_LITERAL %s\n", yytext);
					break;
				case TOKEN_CHAR_LITERAL:
					printf("TOKEN_CHAR_LITERAL %s\n", yytext);
					break;
			}	
		}

	}

	else if(!strcmp(argv[1], "-print")) {
		if(yyparse()==0) {
			decl_print(parser_result, 0);
			return 0;
		} else {
			printf("parse failed!\n");
			return 1;
		}	
	}

	else if(!strcmp(argv[1], "-resolve")) {
		if(yyparse()==0) {
			scope_enter();
			int result = decl_resolve(parser_result, 1);
			scope_exit();
			return !result;
		} else {
			printf("parse failed!\n");
			return 1;
		}
	}

	else if(!strcmp(argv[1], "-typecheck")) {
		if(yyparse()==0) {
			scope_enter();
			int result = decl_resolve(parser_result, 1);
			scope_exit();
			if (!result) {
				return 1;
			}

			result = decl_typecheck(parser_result);
			return !result;
		} else {
			printf("parse failed!\n");
			return 1;
		}
	}
	else if (!strcmp(argv[1], "-codegen")) {
		if(yyparse()==0) {
			scope_enter();
			int result = decl_resolve(parser_result, 1);
			scope_exit();
			if (!result) {
				return 1;
			}

			result = decl_typecheck(parser_result);
			if(result) {
				// Generate the code
				FILE* fp = fopen(argv[3], "w+");

				// Handle opening errors
				if(fp == 0) {
					printf("Error: File %s could not be opened. Exiting...\n", argv[3]);
					return 1;
				}
				fprintf(fp, ".data\n");
				decl_codegen_globals(parser_result, fp);
				fprintf(fp, ".text\n");
				decl_codegen(parser_result, fp);
			}
			else {
				return !result;
			};
		} else {
			printf("parse failed!\n");
			return 1;
		}
	}

	return 0;

}

void usage() {
	printf("Usage: cminor <options> <filename>\n");
	exit(1);
}
