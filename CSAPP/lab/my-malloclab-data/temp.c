#include <stdio.h>
int main() {
	size_t t = 3;
	int p = (size_t)(&t);

	printf("%d %d %d\n", &t, p, *((size_t *)p));
}