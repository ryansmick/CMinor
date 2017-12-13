#include <stdio.h>
#include <stdlib.h>
#include "label.h"
#include "utils.h"

int label_num = 0;

int label_create() {

	int new_label = label_num;
	label_num++;
	return new_label;

}
const char* label_name(int l) {

	int length = digits_in_integer(l);
	
	int bufsize = (length + 3);
	char* label = malloc(sizeof(char) * bufsize);
	snprintf(label, bufsize, ".L%d", l);
	return label;

}


