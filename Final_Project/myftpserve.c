#include "myftp.h"
#define BACK_LOG 4

int init();
int ctrl_conn_loop(int listenfd);

int main(int argc, char **argv)
{
	int listenfd = init();
	int clientfd = ctrl_conn_loop(listenfd);

	// we are now in the child process

	char buffer[MAX_COMMAND_LENGTH] = {0};
	int actual;
	while((actual = read(clientfd, buffer, MAX_COMMAND_LENGTH)) > 0) {
		char response = 0;
		if(!strcmp(buffer, "Q\n")) {
			response = 'A';
			write(clientfd, &response, 1);
			close(clientfd);
			printf("Child %d exiting...\n", getpid());
			exit(0);
		}
	}

	// start listening for control connections
	// read_command(clientfd)

	return 0;
}

// NOTE: the source of this function is taken from my assignment 8 source code
int init() 
{
	int listenfd = socket(AF_INET, SOCK_STREAM, 0);

	if (listenfd == -1) {
		perror("Error: ");
		exit(errno);
	}

	// remove port already in use error by setting socket options
	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &(int) {1},
	    sizeof(int))) {
		perror("Error: ");
		exit(errno);
	}

	// set up server adress info
	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERVER_PORT);
	// just use the wildcard IP
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	// bind socket to ip and port
	if (bind(listenfd, (struct sockaddr *) &serv_addr,
	         sizeof(serv_addr)) < 0) {
		perror("Error: ");
		exit(errno);
	}

	// set socket as listening
	if (listen(listenfd, BACK_LOG)) {
		perror("Error: ");
		exit(errno);
	}
	return listenfd;
}

// NOTE: this function is taken from my assignment 8 source code
int ctrl_conn_loop(int listenfd)
{
	int clientfd;
	struct sockaddr_in client_addr;
	int length = sizeof(client_addr);

	// set up daemon behavior
	while (1) {
		// clear zombie children
		while (waitpid(-1, NULL, WNOHANG) > 0);

		// wait for a connection
		clientfd = accept(listenfd, (struct sockaddr *) &client_addr,
		                  (socklen_t *) &length);

		if (clientfd == -1) {
			perror("Error: ");
			exit(errno);
		}

		if (fork()) {
			// parent doesn't need this
			close(clientfd);
			continue;
		}

		// get the client name from address info
		char client_name[NI_MAXHOST];
		int client_entry;
		client_entry = getnameinfo((struct sockaddr *) &client_addr,
		                           sizeof(client_addr),
		                           client_name,
		                           sizeof(client_name),
		                           NULL,
		                           0,
		                           NI_NUMERICSERV);

		// print client name and total connections if successful
		if (client_entry) {
			fprintf(stderr, "Error: %s\n",
			        gai_strerror(client_entry));
			close(clientfd);
			exit(client_entry);
		} else {
			printf("Child %d: Connection accepted from host %s\n", 
			       getpid(), client_name);
		}

		return clientfd;
	}
}