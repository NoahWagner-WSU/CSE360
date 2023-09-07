#include <stdio.h>

int lineNum(char *dictionaryName, char *word, int dictWidth);

int main() 
{
	printf("%d\n", lineNum("tiny_9", "horse", 9));

	return 0;
}