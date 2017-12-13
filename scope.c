#include "scope.h"
#include "hash_table.h"
#include <stdlib.h>

struct scope* head = NULL;

struct scope* scope_create() {
	struct scope* s = malloc(sizeof(*s));

	s->variables = hash_table_create(0, 0);
	s->level = scope_level() + 1;
	s->next = 0;

	return s;
}

void scope_enter() {
	struct scope* s = scope_create();
	s->next = head;
	head = s;
}

void scope_exit() {
	if (head) {
		struct scope* s = head;
		head = head->next;
		hash_table_delete(s->variables);
	}
}

int scope_level() {
	if (!head) {
		return 0;
	}
	return head->level;
}

void scope_bind(const char* name, struct symbol* symbol) {
	hash_table_insert(head->variables, name, symbol);
}

struct symbol* scope_lookup(const char* name) {
	struct scope* curr = head;
	while(curr) {
		struct symbol* s = (struct symbol*) hash_table_lookup(curr->variables, name);
		if (s) {
			return s;
		}
		curr = curr->next;
	}

	return 0;
}

struct symbol* scope_lookup_current(const char* name) {
	if (!head) {
		return 0;
	}

	return (struct symbol*) hash_table_lookup(head->variables, name);
}
