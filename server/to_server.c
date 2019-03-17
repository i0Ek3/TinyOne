#include "to_server.h"

int main(int argc, char* argv[])
{
    if (argc != 2) {
        perror("Usage: ./tos port\n");
        exit(0);
    }

    int port = atoi(argv[1]);

    int listenfd;
    if ((listenfd = socket_create(port)) < 0) {
        perror("Failed to create socket!");
        exit(1);
    }

    int sock_control;
    int pid;
    for ( ; ; ) {
        if ((sock_control = socket_accept(listenfd)) < 0) {
            break;
        }

        if ((pid = fork()) < 0) {
            perror("Failed to fork child process!");
        } else if (pid == 0) {
            close(listenfd);
            to_server_process(sock_control);
            close(sock_control);
            exit(0);
        }

        close(sock_control);
    }

    close(listenfd);
    return 0;
}

void to_server_retrive(int sock_control, int sock_data, char* filename) {
    FILE* fd = NULL;
    char data[MAXSIZE];
    size_t read_size;

    fd = fopen(filename, "r");
    if (!fd) {
        send_response(sock_control, 550); // 550: requested action not taken
    } else {
        send_response(sock_control, 150); // 150: file status is ok
        do {
            read_size = fread(data, 1, MAXSIZE, fd);
            if (read_size < 0) {
                printf("Error in fread()!\n");
            }
            if (send(sock_data, data, read_size, 0) < 0) {
                perror("Failed to send file!\n");
            }
        } while (read_size > 0);
        send_response(sock_control, 226); // 226: close connection, file transfer successful
        fclose(fd);
    }
}

int to_server_list(int sock_data, int sock_control) {
    char data[MAXSIZE];
    size_t read_size;

    int call_status = system("ls -l > tmp.txt");
    if (call_status < 0) {
        exit(1);
    }

    FILE* fd = fopen("tmp.txt", "r");
    if (!fd) {
        exit(1);
    }

    fseek(fd, SEEK_SET, 0);
    send_response(sock_control, 1);
    memset(data, 0, MAXSIZE);

    while ((read_size = fread(data, 1, MAXSIZE, fd)) > 0) {
        if (send(sock_data, data, read_size, 0) < 0) {
            perror("Error happened!");
            memset(data, 0, MAXSIZE);
        }
    }

    fclose(fd);
    send_response(sock_control, 226);
    return 0;
}

void to_server_push(int sock_data, char* filename) {
    int ack;
    int sock_control = 0;
    //char buf[MAXSIZE];

    if (recv(sock_control, &ack, sizeof(ack), 0) < 0) {
        send_response(sock_control, 502);
        return;
    }

    int status = ntohl(ack);
    if (533 == status) {
        send_response(sock_control, 533);
        return;
    }

    char name[20];
    memset(name, 0, sizeof(name));
    strcpy(name, "ftp://");
    strcat(name, filename);
    int fd = open(name, O_CREAT|O_WRONLY, 0644);
    if (fd < 0) {
        send_response(sock_control, 502);
        return;
    }

    while(1) {
        char data[MAXSIZE];
        memset(data, 0, sizeof(data));
        ssize_t s = recv(sock_data, data, sizeof(data), 0);
        if (s <= 0) {
            if (s < 0) {
                send_response(sock_control, 502); // command failed
            } else {
                send_response(sock_control, 226); // command successful
            }
            break;
        }
        write(fd, data, s);
    }
    close(fd);
}

int to_server_conn(int sock_control) {
    char buf[MAXSIZE];
    int wait, sock_data;

    if (recv(sock_control, &wait, sizeof(wait), 0) < 0) {
        perror ("Error while waiting!");
        return -1;
    }

    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    getpeername(sock_control, (struct sockaddr*)&client_addr, &len);
    inet_ntop(AF_INET, &client_addr.sin_addr, buf, sizeof(buf));

    if ((sock_data = socket_connect(PORT, buf)) < 0) {
        return -1;
    }

    return sock_data;
}

int to_server_check(char* user, char* passwd) {
    char username[MAXSIZE];
    char password[MAXSIZE];
    char buf[MAXSIZE];
    char* pch;
    char* line = NULL;
    size_t read_size;
    size_t len = 0;
    FILE* fd;
    int auth = 0;

    fd = fopen("auth.txt", "r");
    if (NULL == fd) {
        perror("404 File Not Found!");
        exit(1);
    }

    while ((read_size = getline(&line, &len, fd)) != -1) {
        memset(buf, 0, MAXSIZE);
        strcpy(buf, line);

        pch = strtok(buf, " "); // split string in buf by space
        strcpy(username, pch);

        if (pch != NULL) {
            pch = strtok(NULL, " ");
            strcpy(password, pch);
        }

        trim_char(password, (int)strlen(password));

        if ((strcmp(user, username) == 0) && (strcmp(passwd, password)) == 0) {
            auth = 1;
            break;
        }
    }
    free(line);
    fclose(fd);
    return auth;
}

int to_server_login(int sock_control) {
    char buf[MAXSIZE];
    char user[MAXSIZE];
    char passwd[MAXSIZE];
    memset(buf, 0, MAXSIZE);
    memset(user, 0, MAXSIZE);
    memset(passwd, 0, MAXSIZE);

    if ((recv_data(sock_control, buf, sizeof(buf))) == -1) {
        perror("Failed to receive data!\n");
        exit(1);
    }

    int i = 5;
    int n = 0;
    while (buf[i] != 0) {
        user[n++] = buf[i++];
    }

    send_response(sock_control, 331);
    memset(buf, 0, MAXSIZE);

    if ((recv_data(sock_control, buf, sizeof(buf))) == -1) {
        perror("Failed to receive data!\n");
        exit(1);
    }

    i = 5;
    n = 0;
    while (buf[i] != 0) {
        passwd[n++] = buf[i++];
    }

    return (to_server_check(user, passwd));
}

int to_server_recv_cmd(int sock_control, char* cmd, char* arg) {
    int retcode = 200;
    char buf[MAXSIZE];

    memset(buf, 0, MAXSIZE);
    memset(cmd, 0, 5);
    memset(arg, 0, MAXSIZE);

    if ((recv_data(sock_control, buf, sizeof(buf))) == -1) {
        perror("Failed to receive data!\n");
        return -1;
    }

    strncpy(cmd, buf, 4);
    char* tmp = buf + 5;
    strcpy(arg, tmp);
    if (strcmp(cmd, "QUIT") == 0) {
        retcode = 221;
    } else if (strcmp(cmd, "USER") == 0 || (strcmp(cmd, "PASS")) == 0 || (strcmp(cmd, "LIST")) == 0 || (strcmp(cmd, "RETR")) == 0 || (strcmp(cmd, "PUSH")) == 0) {
        retcode = 200;
    } else {
        retcode = 502;
    }

    send_response(sock_control, retcode);
    return retcode;
}

void to_server_process(int sock_control) {
    int sock_data;
    char cmd[5];
    char arg[MAXSIZE];

    send_response(sock_control, 220);
    if (to_server_login(sock_control) == 1) {
        send_response(sock_control, 230); // auth is ok
    } else {
        send_response(sock_control, 430); // auth is failed
        exit(0);
    }

    while (1) {
        int retcode = to_server_recv_cmd(sock_control, cmd, arg);
        if ((retcode < 0) || (retcode == 221)) {
            break;
        }

        if (retcode == 200) {
            if ((sock_data = to_server_conn(sock_control)) < 0) {
                close(sock_control);
                exit(1);
            }

            if (strcmp(cmd, "LIST") == 0) {
                to_server_list(sock_data, sock_control);
            } else if (strcmp(cmd, "RETR") == 0) {
                to_server_retrive(sock_control, sock_data, arg);
            } else if (strcmp(cmd, "PUSH") == 0) {
                to_server_retrive(sock_control, sock_data, arg);
            }
            close(sock_data);
        }
    }
}
