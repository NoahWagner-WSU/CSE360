#include <stdio.h>

int lineNum(char *dictionaryName, char *word, int dictWidth);

int main() 
{
	printf("m:167 s:%d\n", lineNum("webster_16", "acid", 16));
	printf("m:-1 s:%d\n", lineNum("tiny_9", "acid", 9));
	printf("m:11609 s:%d\n", lineNum("webster_16", "mellow", 16));
	printf("m:10 s:%d\n", lineNum("tiny_9", "mellow", 9));
	printf("m:20413 s:%d\n", lineNum("webster_16", "youth", 16));
	printf("m:-10 s:%d\n", lineNum("tiny_9", "youth", 9));
	printf("m:173 s:%d\n", lineNum("webster_16", "acknowledgeable", 16));
	printf("m:-1 s:%d\n", lineNum("tiny_9", "acknowledgeable", 9));
	printf("m:173 s:%d\n", lineNum("webster_16", "acknowledgeables", 16));
	printf("m:-1 s:%d\n", lineNum("tiny_9", "acknowledgeables", 9));
	printf("m:20422 s:%d\n", lineNum("webster_16", "zoo", 16));
	printf("m:-10 s:%d\n", lineNum("tiny_9", "zoo", 9));
	printf("m:16 s:%d\n", lineNum("webster_16", "abc", 16));
	printf("m:-1 s:%d\n", lineNum("tiny_9", "abc", 9));
	printf("m:-7048 s:%d\n", lineNum("webster_16", "fi sh", 16));
	printf("m:7 s:%d\n", lineNum("tiny_9", "fi sh", 9));
	printf("m:-1 s:%d\n", lineNum("webster_16", "000", 16));
	printf("m:-1 s:%d\n", lineNum("tiny_9", "000", 9));
	printf("m:1 s:%d\n", lineNum("webster_16", "a b c", 16));
	printf("m:-1 s:%d\n", lineNum("tiny_9", "a b c", 9));

	return 0;
}