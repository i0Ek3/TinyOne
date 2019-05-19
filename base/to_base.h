#ifndef TO_BASE_H
#define TO_BASE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
//#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/uio.h> // for macOS
#include <sys/types.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <ctype.h>

#define BACKLOG 5
#define MAXSIZE 1024
#define PORT 9999

struct command {
    char arg[255];
    char code[5];
};

int socket_create(int port);
int socket_accept(int listenfd);
int socket_connect(const char* host, const int port);
int recv_data(int sockfd, char* buf, int bufsize);
int send_response(int sockfd, int retcode);
void read_input(char* buf, int size);
void trim_char(char* str, int n);

#endif
