all: cminor

cminor: scanner.c parser.tab.c main.c
	/usr/bin/gcc -Wall -Wno-unused-label main.c scanner.c parser.tab.c decl.c stmt.c expr.c type.c param_list.c symbol.c scope.c hash_table.c scratch.c label.c utils.c -o cminor

debug: scanner.c parser.tab.c main.c
	/usr/bin/gcc -Wall -Wno-unused-label -g main.c scanner.c parser.tab.c decl.c stmt.c expr.c type.c param_list.c symbol.c scope.c hash_table.c scratch.c label.c utils.c -o cminor_debug

scanner.c: scanner.flex
	flex -o scanner.c scanner.flex

parser.tab.c parser.tab.h: parser.bison
	bison --defines=parser.tab.h --output=parser.tab.c -v parser.bison

clean:
	rm -f cminor cminor_debug scanner.c parser.tab.c parser.tab.h parser.output

