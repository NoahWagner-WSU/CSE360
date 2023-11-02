#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <time.h>

#define SERVER_PORT 49999

void server();
void client(char *address);

int main(int argc, char const *argv[])
{
	// parse argv here, run client / server depending on argv[1]
	if(argc == 1) {
		fprintf(stderr, "%s\n", 
			"Usage: ./assignment8 <client/server> <address>");
		return 1;
	}

	if(strcmp(argv[1], "server") == 0) {
		server();
	} else if(strcmp(argv[1], "client") == 0) {
		if(argc >= 2) {
			client(argv[2]);
		} else {
			fprintf(stderr, "%s\n", 
				"Error: client must recieve server adress");
			return 1;
		}
	}
    return 0;
}