#include "cachelab.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define debug(x)                                                                                                                           \
	if (debugMod) { printf("%s\t=%d\n", (#x), x); }
#define sdebug(x)                                                                                                                          \
	if (debugMod) { printf("%s\t=%s\n", (#x), x); }

int debugMod;
int s, E, b;
int miss, hit, eviction;

int main(int argc, char **argv) {
	char command;
	char fileName[20];
	while ((command = getopt(argc, argv, "vs:E:b:t:")) != -1) {
		if (command == 'v') {
			debugMod = 1;
			debug(debugMod);
		} else if (command == 't') {
			strcpy(fileName, optarg);
			sdebug(fileName);
		} else {
			int value = atoi(optarg);
			if (command == 's') s = value;
			if (command == 'E') E = value;
			if (command == 'b') b = value;
			debug(s);
			debug(E);
			debug(b);
		}
	}

	printSummary(hit, miss, eviction);
	return 0;
}
