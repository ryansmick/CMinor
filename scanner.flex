 /* Ryan Smick
 * scanner.flex
 * Flex file defining scanner for cminor language
 */


%option nounput
%option noinput

%{
	#include "parser.tab.h"

	char* original_literal;
%}

NUMBER [0-9]
LETTER [a-zA-Z]

%%

[/][*]([^*]|[*][^/])*[*]+[/] /* Ignore comments */
[/][/][^\n]* /* Ignore comments */
<<EOF>> {return TOKEN_EOF;}
array {return TOKEN_ARRAY;}
boolean {return TOKEN_BOOLEAN;}
char {return TOKEN_CHAR;}
else {return TOKEN_ELSE;}
false {return TOKEN_FALSE;}
for {return TOKEN_FOR;}
function {return TOKEN_FUNCTION;}
if {return TOKEN_IF;}
integer {return TOKEN_INTEGER;}
print {return TOKEN_PRINT;}
return {return TOKEN_RETURN;}
string {return TOKEN_STRING;}
true {return TOKEN_TRUE;}
void {return TOKEN_VOID;}
while {return TOKEN_WHILE;}
[[] {return TOKEN_LEFT_BRACKET;}
[]] {return TOKEN_RIGHT_BRACKET;}
[(] {return TOKEN_LEFT_PAREN;}
[)] {return TOKEN_RIGHT_PAREN;}
[{] {return TOKEN_LEFT_CURLY;}
[}] {return TOKEN_RIGHT_CURLY;}
\+\+ {return TOKEN_INCREMENT;}
\+ {return TOKEN_PLUS;}
-- {return TOKEN_DECREMENT;}
- {return TOKEN_MINUS;}
!= {return TOKEN_NE;}
! {return TOKEN_NOT;}
\^ {return TOKEN_XOR;}
\* {return TOKEN_MULT;}
\/ {return TOKEN_DIVIDE;}
% {return TOKEN_MODULUS;}
\<= {return TOKEN_LE;}
\< {return TOKEN_LT;}
>= {return TOKEN_GE;}
> {return TOKEN_GT;}
== {return TOKEN_EQUAL;}
= {return TOKEN_ASSIGN;}
&& {return TOKEN_AND;}
\|\| {return TOKEN_OR;}
: {return TOKEN_COLON;}
; {return TOKEN_SEMICOLON;}
, {return TOKEN_COMMA;}
[ \n\t\r]+ /* Ignore Whitespace */
({LETTER}|[_])({NUMBER}|{LETTER}|[_])* {
	if(strlen(yytext) <= 256) {
		return TOKEN_ID;
	}
	else {
		fprintf(stderr, "Scan error: identifier %s greater than 256 characters in length\n", yytext);
		exit(1);
	}
}
{NUMBER}+ {return TOKEN_INTEGER_LITERAL;}
["]([^\n"]|\\.)*["] {
	original_literal = strdup(yytext);
	char newString[strlen(yytext)];
	int i = 0;
	int j = 0;
	while(yytext[i] != '\0') {
		if(yytext[i] == '\\' && yytext[i+1] == 'n') {
			newString[j] = '\n';
			i += 2;
			j += 2;
		}
		else if(yytext[i] == '\\' && yytext[i+1] == '0') {
			newString[j] = '\0';
			i += 2;
			j += 2;
		}
		else if(yytext[i] == '\\' && yytext[i+1] == '"') {
			newString[j] = '"';
			i += 2;
			j += 2;
		}
		else if(yytext[i] == '\\' || yytext[i] == '"') {
			i++;
			continue;
		}
		else {
			newString[j] = yytext[i];
			i++;
			j++;
		}
	}
	// Add null terminator to end
	newString[j] = '\0';

	if(strlen(newString) < 256) {
		yytext = strdup(newString);
		return TOKEN_STRING_LITERAL;
	}
	else {
		fprintf(stderr, "Scan error: string literal %s is greater than 255 characters long\n", yytext);
		exit(1);
	}
}
[']([^'\\\n]|\\.)['] {
	original_literal = strdup(yytext);
	char newChar[2];
	if(strlen(yytext) == 3) {
		newChar[0] = yytext[1];
		newChar[1] = '\0';
		yytext = newChar;
		return TOKEN_CHAR_LITERAL;
	}
	else {
		if(yytext[2] == 'n') {
			newChar[0] = '\n';
		}
		else if(yytext[2] == '0') {
			newChar[0] = '\0';
		}
		else {
			newChar[0] = yytext[2];
		}
		newChar[1] = '\0';
		return TOKEN_CHAR_LITERAL;		
	}
}
. {return TOKEN_ERROR;}

%%

int yywrap() {
	return 1;
}
