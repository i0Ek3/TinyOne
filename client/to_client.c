#include "to_client.h"

int sock_control;

int main(int argc, char* argv[])
{
    int sock_data, retcode, s;
    char buf[MAXSIZE];
    struct command cmd;
    struct addrinfo hints, *res, *rp;

    if (argc != 3) {
        printf("Usage: ./toc ip port");
        exit(0);
    }

    char* ip = argv[1];
    char* port = argv[2];

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    s = getaddrinfo(ip, port, &hints, &res);
    if (s != 0) {
        printf("getaddrinfo() error %s", gai_strerror(s));
        exit(1);
    }

    for (rp = res; rp != NULL; rp = rp->ai_next) {
        sock_control = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

        if (sock_control < 0) {
            continue;
        }
        if (connect(sock_control, res->ai_addr, res->ai_addrlen) == 0) {
            break;
        } else {
            perror("Connecting stream socket.");
            exit(1);
        }
        close(sock_control);
    }
    freeaddrinfo(rp);

    printf("Connected to %s.\n", ip);
    print_reply(read_reply());

    to_client_login();

    while (1) {
        if (to_client_read_cmd(buf, sizeof(buf), &cmd) < 0) {
            printf("Invalid command.\n");
            continue;
        }
        if (send(sock_control, buf, (int)strlen(buf), 0) < 0) {
            close(sock_control);
            exit(1);
        }

        retcode = read_reply();
        if (retcode == 221) {
            print_reply(221);
            break;
        }
        if (retcode == 502) {
            printf("%d Invalid command.\n", retcode);
        } else {
            if ((sock_data = to_client_open_conn(sock_control)) < 0) {
                perror("Error while opening socket for data connection!");
                exit(1);
            }
            if (strcmp(cmd.code, "LIST") == 0) {
                to_client_list(sock_data, sock_control);
            } else if (strcmp(cmd.code, "RETR") == 0) {
                if (read_reply() == 550) {
                    print_reply(550);
                    close(sock_data);
                    continue;
                }
                to_client_get(sock_data, sock_control, cmd.arg);
                print_reply(read_reply());
            }
            close(sock_data);
        }
    }
    close(sock_control);
    return 0;
}

int read_reply() {
    int retcode = 0;
    if (recv(sock_control, &retcode, sizeof(retcode), 0) < 0) {
        perror("Client: Failed reading data from server.\n");
        return -1;
    }
    return ntohl(retcode);
}

void print_reply(int retcode) {
    switch(retcode) {
        case 220:
            printf("220 Welcome, server ready!\n");
            break;
        case 221:
            printf("221 See you next time.\n");
            break;
        case 226:
            printf("226 Closing data connection, requested file action successful.\n");
            break;
        case 550:
            printf("550 Requested action not taken. File unavailable.\n");
            break;
    }
}

int to_client_read_cmd(char* buf, int size, struct command* cmd) {
    memset(cmd->code, 0, sizeof(cmd->code));
    memset(cmd->arg, 0, sizeof(cmd->arg));

    printf("toc>");
    fflush(stdout);
    read_input(buf, size);

    char* arg = NULL;
    arg = strtok(buf, " ");
    arg = strtok(NULL, " ");

    if (arg != NULL) {
        strncpy(cmd->arg, arg, strlen(arg));
    }

    if (strcmp(buf, "list") == 0) {
        strcpy(cmd->code, "LIST");
    } else if (strcmp(buf, "get") == 0) {
        strcpy(cmd->code, "RETR");
    } else if (strcmp(buf, "quit") == 0) {
        strcpy(cmd->code, "QUIT");
    } else {
        return -1;
    }

    memset(buf, 0, 400);
    strcpy(buf, cmd->code);

    if (arg != NULL) {
        strcat(buf, " ");
        strncat(buf, cmd->arg, strlen(cmd->arg));
    }

    return 0;
}

int to_client_get(int sock_data, int sock_control, char* arg) {
    char data[MAXSIZE];
    int size;
    FILE* fd = fopen(arg, "w");
    while ((size = recv(sock_data, data, MAXSIZE, 0)) > 0) {
        fwrite(data, 1, size, fd);
    }
    if (size < 0) {
        perror("Error!\n");
    }
    fclose(fd);
    return 0;
}

int to_client_open_conn(int sock_con) {
    int listenfd = socket_create(PORT);
    int ack = 1;
    if ((send(sock_con, (char*)&ack, sizeof(ack), 0)) < 0) {
        printf("Client: ack write error: %d\n", errno);
        exit(1);
    }

    int sock_conn = socket_accept(listenfd);
    close(listenfd);
    return sock_conn;
}

int to_client_list(int sock_data, int sock_conn) {
    size_t read_size;
    char buf[MAXSIZE];
    int tmp = 0;

    if (recv(sock_conn, &tmp, sizeof(tmp), 0) < 0) {
        perror("Client: error while reading data from server.\n");
        return -1;
    }

    memset(buf, 0, sizeof(buf));

    while((read_size = recv(sock_data, buf, MAXSIZE, 0)) > 0) {
        printf("%s", buf);
        memset(buf, 0, sizeof(buf));
    }

    if (read_size < 0) {
        perror("Error!");
    }

    if (recv(sock_conn, &tmp, sizeof tmp, 0) < 0) {
        perror("Client: error while reading data from server.\n");
        return -1;
    }

    return 0;
}

int to_client_send_cmd(struct command* cmd) {
    char buf[MAXSIZE];
    int retcode;
    sprintf(buf, "%s %s", cmd->code, cmd->arg);
    retcode = send(sock_control, buf, (int)strlen(buf), 0);
    if (retcode < 0) {
        perror("Error while sending command to server.\n");
        return -1;
    }

    return 0;
}

void to_client_login() {
    struct command cmd;
    char user[256];
    memset(user, 0, 256);

    printf("username: ");
    fflush(stdout);
    read_input(user, 256);

    strcpy(cmd.code, "USER");
    strcpy(cmd.arg, user);
    to_client_send_cmd(&cmd);

    int wait;
    recv(sock_control, &wait, sizeof wait, 0);

    fflush(stdout);
    char* passwd = getpass("password: ");

    strcpy(cmd.code, "PASS");
    strcpy(cmd.arg, passwd);
    to_client_send_cmd(&cmd);

    int retcode = read_reply();
    switch (retcode) {
        case 430:
            printf("Invalid username or password.\n");
            exit(0);
        case 230:
            printf("Login successfully.\n");
            break;
        default:
            perror("Error while reading data from server.\n");
            exit(1);
            break;
    }
}
