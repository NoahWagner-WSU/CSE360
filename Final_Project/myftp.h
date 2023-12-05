#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <time.h>
#include <limits.h>
#include <libgen.h>
#include <fcntl.h>
#include <sys/stat.h>

// move all shared function declarations here

#define SERVER_PORT 49999

#define READ_BUFFER_SIZE 1024

char *get_line(int fd);
void send_msg(int ctrl_sock, char type, char *msg);
int copy(int src, int dst);