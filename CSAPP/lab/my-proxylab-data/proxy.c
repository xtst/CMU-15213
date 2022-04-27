#include "csapp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define MAX_STRING_SIZE 12345
/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *error = "Error!";
int main(int argc, char **argv) {
	char s[MAX_STRING_SIZE];
	int t = 0;
	if (argc == 2)
		t = atoi(argv[1]);
	else {
		t = 555;
		// printf("%s", error);
		// return -1;
	}
	// printf("%d\n", t);
	printf("%s", user_agent_hdr);
	while (getline(s, MAX_STRING_SIZE, stdin)) { printf("%s", s); }
	// rio
	return 0;
}
