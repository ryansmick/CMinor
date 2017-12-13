int digits_in_integer(int x) {

	// Determine number of digits in the given number
	int length = 0;
	do {
		x = x/10;
		length++;
	} while(x != 0);

	return length;
}
