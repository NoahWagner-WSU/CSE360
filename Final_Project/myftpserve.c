#include "myftp.h"
#include <signal.h>

#define BACK_LOG 4

int init_socket(int port, int back_log, int *assigned_port);
int ctrl_conn_loop(int listenfd);

void handle_Q(int clientfd);
void handle_C(int clientfd, char *path);
int handle_D(int clientfd);
void handle_L(int clientfd, int datafd);
void handle_G(int clientfd, int datafd, char *path);
void handle_P(int clientfd, int datafd, char *path);

int main(int argc, char **argv)
{
	int listenfd = init_socket(SERVER_PORT, BACK_LOG, NULL);

	if (listenfd == -1) {
		return errno;
	}

	int clientfd = ctrl_conn_loop(listenfd);

	char *line;
	int datafd = -1;
	while ((line = get_line(clientfd)) != NULL) {

		if(line[0] == 'D' && datafd != -1) {
			fprintf(stderr, 
			        "Child %d: recieved redundant D command, exiting\n",
			        getpid());
			send_msg(clientfd, 'E', "Unexpected D command");
			exit(EXIT_FAILURE);
		}

		if ((line[0] == 'L' || line[0] == 'G' || line[0] == 'P') &&
		    datafd == -1) {
			send_msg(clientfd, 'E',
			         "No data connection established");
			free(line);
			continue;
		}

		if (line[0] == 'Q') {
			free(line);
			handle_Q(clientfd);
		} else if (line[0] == 'C') {
			handle_C(clientfd, line + 1);
		} else if (line[0] == 'D') {
			datafd = handle_D(clientfd);
		} else if (line[0] == 'L') {
			handle_L(clientfd, datafd);
			datafd = -1;
		} else if (line[0] == 'G') {
			handle_G(clientfd, datafd, line + 1);
			datafd = -1;
		} else if (line[0] == 'P') {
			handle_P(clientfd, datafd, line + 1);
			datafd = -1;
		} else {
			send_msg(clientfd, 'E',
			         "Unrecognized control command");
		}
		free(line);
	}

	fprintf(stderr,
	        "Child %d: Failed to read from Control Socket, exiting\n",
	        getpid());
	return EXIT_FAILURE;
}

// NOTE: the source of this function is taken from my assignment 8 source code
int init_socket(int port, int back_log, int *assigned_port)
{
	int listenfd = socket(AF_INET, SOCK_STREAM, 0);

	if (listenfd == -1) {
		fprintf(stderr, "Socket Error: %s\n", strerror(errno));
		return -1;
	}

	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &(int) {1},
	    sizeof(int))) {
		fprintf(stderr, "Socket Option Error: %s\n", strerror(errno));
		return -1;
	}

	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(listenfd, (struct sockaddr *) &serv_addr,
	                sizeof(serv_addr)) < 0) {
		fprintf(stderr, "Bind Error: %s\n", strerror(errno));
		return -1;
	}

	if (port == 0) {
		struct sockaddr_in serv_addr;
		memset(&serv_addr, 0, sizeof(serv_addr));
		int length = sizeof(serv_addr);
		if (getsockname(listenfd, (struct sockaddr *) &serv_addr,
		                (socklen_t *) &length)) {
			fprintf(stderr, "Socket Name Error: %s\n", 
			        strerror(errno));
			return -1;
		}
		if (assigned_port)
			*assigned_port = ntohs(serv_addr.sin_port);
	}

	if (listen(listenfd, back_log)) {
		fprintf(stderr, "Listen Error: %s\n", strerror(errno));
		return -1;
	}

	return listenfd;
}

// NOTE: this function is taken from my assignment 8 source code
int ctrl_conn_loop(int listenfd)
{
	int clientfd;
	struct sockaddr_in client_addr;
	int length = sizeof(client_addr);

	while (1) {
		while (waitpid(-1, NULL, WNOHANG) > 0);

		clientfd = accept(listenfd, (struct sockaddr *) &client_addr,
		                  (socklen_t *) &length);

		if (clientfd == -1) {
			fprintf(stderr, "Fatal Accept Error: %s\n", 
			        strerror(errno));
			exit(errno);
		}

		if (fork()) {
			close(clientfd);
			continue;
		}

		char client_name[NI_MAXHOST];
		int client_entry;
		client_entry = getnameinfo((struct sockaddr *) &client_addr,
		                           sizeof(client_addr),
		                           client_name,
		                           sizeof(client_name),
		                           NULL,
		                           0,
		                           NI_NUMERICSERV);

		if (client_entry) {
			fprintf(stderr, 
			        "Child %d Get Name Error: %s, exiting\n",
				getpid(),
			        gai_strerror(client_entry));
			exit(client_entry);
		} else {
			printf("Child %d: Connection accepted from host %s\n",
			       getpid(), client_name);
		}

		return clientfd;
	}
}

char *get_line(int fd)
{
	char *result = NULL;
	int result_size = 0;

	char buffer[READ_BUFFER_SIZE] = {0};
	int actual = 0;

	while ((actual = read(fd, buffer, READ_BUFFER_SIZE)) > 0) {

		char *tmp = calloc(result_size + actual, 1);

		if (result) {
			memcpy(tmp, result, result_size);
			free(result);
		}
		memcpy(tmp + result_size, buffer, actual);

		result = tmp;
		result_size += actual;

		int i = 0;
		while (result[i] != '\n' && i < result_size)
			i++;
		if (i < result_size) {
			result[i] = '\0';
			break;
		}
		memset(buffer, 0, READ_BUFFER_SIZE);
	}

	if (actual < 0) {
		if (result)
			free(result);
		return NULL;
	}

	return result;
}

void send_msg(int ctrl_sock, char type, char *msg)
{
	int tmp_errno;
	if (write(ctrl_sock, &type, 1) == -1) {
		tmp_errno = errno;
		fprintf(stderr,
		        "Fatal Write Error: %s\n",
		        strerror(tmp_errno));
		exit(tmp_errno);
	}

	if (msg) {
		if (write(ctrl_sock, msg, strlen(msg)) == -1) {
			tmp_errno = errno;
			fprintf(stderr,
			        "Fatal Write Error: %s\n",
			        strerror(tmp_errno));
			exit(tmp_errno);
		}
	}

	char new_line = '\n';

	if (write(ctrl_sock, &new_line, 1) == -1) {
		tmp_errno = errno;
		fprintf(stderr,
		        "Fatal Write Error: %s\n",
		        strerror(tmp_errno));
		exit(tmp_errno);
	}
}

int copy(int src, int dst)
{
	int actual;
	char buffer[READ_BUFFER_SIZE] = {0};
	while ((actual = read(src, buffer, READ_BUFFER_SIZE)) > 0) {
		if(write(dst, buffer, actual) == -1) {
			fprintf(stderr, "Write Error: %s\n", strerror(errno));
			break;
		}
	}
	return actual;
}

void handle_Q(int clientfd)
{
	send_msg(clientfd, 'A', NULL);
	close(clientfd);
	printf("Child %d: Quitting\n", getpid());
	exit(0);
}

void handle_C(int clientfd, char *path)
{
	if (access(path, R_OK)) {
		int tmp = errno;
		fprintf(stderr, "Child %d Change Directory Error: %s\n", 
		        getpid(), 
		        strerror(tmp));
		send_msg(clientfd, 'E', strerror(tmp));
		return;
	}

	if (chdir(path)) {
		int tmp = errno;
		fprintf(stderr, "Child %d Change Directory Error: %s\n", 
		        getpid(), 
		        strerror(tmp));
		send_msg(clientfd, 'E', strerror(tmp));
		return;
	}

	printf("Child %d: Changed directory to %s\n", getpid(), path);
	send_msg(clientfd, 'A', NULL);
}

int handle_D(int clientfd)
{
	int port_num = 0;
	int listenfd = init_socket(0, 1, &port_num);

	if (listenfd == -1) {
		send_msg(clientfd, 'E', "Failed to initialize data socket");
		return -1;
	}

	char port[NI_MAXSERV] = {0};
	sprintf(port, "%d", port_num);

	send_msg(clientfd, 'A', port);

	struct sockaddr_in client_addr;
	int length = sizeof(client_addr);

	int datafd = accept(listenfd, (struct sockaddr *) &client_addr,
	                    (socklen_t *) &length);
	close(listenfd);
	if (datafd == -1)
		fprintf(stderr, 
		        "Child %d Failed to establish Data Connection: %s\n",
		        getpid(), 
		        strerror(errno));
	return datafd;
}

void handle_L(int clientfd, int datafd)
{
	int error;

	send_msg(clientfd, 'A', NULL);
	printf("Child %d: Sending ls content\n", getpid());

	int f1 = fork();

	if (f1 > 0) {
		close(datafd);
		wait(NULL);
		return;
	} else if (f1 == -1) {
		int tmp = errno;
		fprintf(stderr, "Child %d Fork error: %s\n", getpid(), 
		        strerror(tmp));
		exit(tmp);
	}
	
	close(1); dup(datafd); close(datafd);
	execlp("ls", "ls", "-l", (char *) NULL);
	int tmp = errno;
	fprintf(stderr, "Child %d Exec Error: %s\n", getpid(), strerror(tmp));
	exit(tmp);
}

void handle_G(int clientfd, int datafd, char *path)
{
	if (access(path, R_OK)) {
		send_msg(clientfd, 'E', strerror(errno));
		close(datafd);
		return;
	}

	struct stat s;
	if (lstat(path, &s)) {
		send_msg(clientfd, 'E', strerror(errno));
		close(datafd);
		return;
	}

	if (!S_ISREG(s.st_mode)) {
		send_msg(clientfd, 'E', "File is not regular");
		close(datafd);
		return;
	}

	send_msg(clientfd, 'A', NULL);

	int openfd = open(path, O_RDONLY);

	if (openfd == -1) {
		fprintf(stderr, "Child %d File Open Error: %s\n", getpid(), 
		        strerror(errno));
		close(datafd);
		return;
	}

	printf("Child %d: Transmitting contents of file %s\n", getpid(), path);

	signal(SIGPIPE, SIG_IGN);

	if (copy(openfd, datafd) < 0)
		fprintf(stderr, "Read Error: %s\n", strerror(errno));

	signal(SIGPIPE, SIG_DFL);

	close(datafd);
}

void handle_P(int clientfd, int datafd, char *path)
{
	char *path_copy = strdup(path);
	char *dir = dirname(path_copy);

	if(strcmp(dir, ".")) {
		send_msg(clientfd, 'E', 
		         "Recieved a path name, base name expected");
		free(path_copy);
		return;
	}

	free(path_copy);

	int newfd = open(path, O_CREAT | O_EXCL | O_WRONLY, 0644);

	if (newfd == -1) {
		send_msg(clientfd, 'E', strerror(errno));
		close(datafd);
		return;
	}

	send_msg(clientfd, 'A', NULL);

	printf("Child %d: Recieving contents for file %s\n", getpid(), path);

	if (copy(datafd, newfd) < 0) {
		fprintf(stderr, "Read Error: %s\n", strerror(errno));
		unlink(path);
	}

	close(datafd); close(newfd);
}