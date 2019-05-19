#include "to_client.h"


int read_reply(int sock_control) { // 读取服务器的回复
    int retcode = 0;
    if (recv(sock_control, &retcode, sizeof(retcode), 0) < 0) {
        perror("Client: Failed reading data from server.\n");
        return -1;
    }
    return ntohl(retcode);
}

void print_reply(int retcode) { // 打印回复消息
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

// 读取客户端输入的命令
int to_client_read_cmd(char* buf, int size, struct command* cmd) { // 从缓冲区中读取命令
    memset(cmd->code, 0, sizeof(cmd->code));
    memset(cmd->arg, 0, sizeof(cmd->arg));

    printf("toc>");
    fflush(stdout);
    read_input(buf, size); // 从buf读取命令

    char* arg = NULL;
    arg = strtok(buf, " "); // 以某个字符作为分隔符
    arg = strtok(NULL, " "); // 指向输入的命令所带的参数

    if (arg != NULL) {
        strncpy(cmd->arg, arg, strlen(arg));
    }

    // 判断命令接口
    if (strcmp(buf, "list") == 0 || strcmp(buf, "ls") == 0) {
        strcpy(cmd->code, "LIST");
    } else if (strcmp(buf, "get") == 0 || strcmp(buf, "download") == 0) {
        strcpy(cmd->code, "RETR");
    } else if (strcmp(buf, "put") == 0 || strcmp(buf, "upload") == 0){
        strcpy(cmd->code, "PUSH");
    } else if (strcmp(buf, "quit") == 0 || strcmp(buf, "q") == 0) {
        strcpy(cmd->code, "QUIT");
    } else {
        return -1;
    }

    // 将命令存到buf中
    memset(buf, 0, 400);
    strcpy(buf, cmd->code);

    if (arg != NULL) {
        strcat(buf, " ");
        strncat(buf, cmd->arg, strlen(cmd->arg)); // 如果带有参数，追加到命令后面
    }

    return 0;
}

int to_client_get(int sock_data, char* filename) { // download
    char data[MAXSIZE];
    int size;
    FILE* fd = fopen(filename, "w");
    while ((size = recv(sock_data, data, MAXSIZE, 0)) > 0) {
        fwrite(data, 1, size, fd);
    }
    if (size < 0) {
        perror("Download error!\n");
    }
    fclose(fd);
    return 0;
}

int to_client_put(int sock_data, char* filename) { // upload
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        return -1;
    }
    struct stat stat_buf;
    if (stat(filename, &stat_buf) < 0) {
        return -1;
    }

    // 6 parametes for macOS, 4 parameters for linux
    // sendfile(int, int, off_t, off_t *, struct sf_hdtr *, int);
    // sendfile(sock_data, fd, NULL, stat_buf.st_size); // for Linux
    sendfile(sock_data, fd, 0, &stat_buf.st_size, NULL, 0); // for macOS
    close(fd);
    return 0;
}

int to_client_open_conn(int sock_control) { // 打开连接
    int listenfd = socket_create(PORT);
    int ack = 1;
    if ((send(sock_control, (char*)&ack, sizeof(ack), 0)) < 0) { // 发送应答
        printf("Client: ack write error: %d\n", errno);
        exit(1);
    }

    int sock_conn = socket_accept(listenfd);
    close(listenfd);
    return sock_conn;
}

int to_client_list(int sock_data, int sock_control) { // list 命令实现
    size_t read_size;
    char buf[MAXSIZE];
    int tmp = 0;

    if (recv(sock_control, &tmp, sizeof(tmp), 0) < 0) {
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

    if (recv(sock_control, &tmp, sizeof tmp, 0) < 0) {
        perror("Client: error while reading data from server.\n");
        return -1;
    }

    return 0;
}

int to_client_send_cmd(int sock_control, struct command* cmd) { // 发送命令
    char buf[MAXSIZE];
    sprintf(buf, "%s %s", cmd->code, cmd->arg); // 将命令和参数读入到缓冲区中
    int retcode = send(sock_control, buf, (int)strlen(buf), 0);
    if (retcode < 0) {
        perror("Error while sending command to server.\n");
        return -1;
    }

    return 0;
}

int to_client_login(int sock_control) { // 登录接口
    struct command cmd;
    char user[256];
    memset(user, 0, 256);

    printf("username: ");
    fflush(stdout);
    read_input(user, 256); // 读取用户名

    strcpy(cmd.code, "USER");
    strcpy(cmd.arg, user);
    to_client_send_cmd(sock_control, &cmd); // 发送用户名到服务器

    int wait;
    recv(sock_control, &wait, sizeof wait, 0);

    fflush(stdout); // 刷新输出
    char* passwd = getpass("password: "); // 获取密码

    strcpy(cmd.code, "PASS");
    strcpy(cmd.arg, passwd);
    to_client_send_cmd(sock_control, &cmd); // 发送密码到服务器

    int retcode = read_reply(sock_control); // 读取服务器的回应
    switch (retcode) { // 校验返回码
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

    return 0;
}

int main(int argc, char* argv[])
{
    int sock_data, retcode;
    char buf[MAXSIZE];
    struct command cmd;
    memset(&cmd, 0, sizeof(cmd));

    if (argc != 3) { // 参数检查
        printf("Usage: ./toc ip port");
        exit(0);
    }

    int sock_control = socket_connect(argv[1], atoi(argv[2]));
    if (sock_control < 0) {
        printf("Connected failed!\n");
        exit(0);
    }

    char* ip = argv[1];

    printf("Connected to %s.\n", ip);
    print_reply(read_reply(sock_control));
    if (to_client_login(sock_control) < 0) {
        exit(0);
    }

    while (1) {
        if (to_client_read_cmd(buf, sizeof(buf), &cmd) < 0) {
            printf("Invalid command.\n");
            continue;
        }
        if (send(sock_control, buf, (int)strlen(buf), 0) < 0) { // 发送命令到服务器
            close(sock_control);
            exit(1);
        }

        retcode = read_reply(sock_control);
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
                close(sock_data);
            } else if (strcmp(cmd.code, "RETR") == 0) {
                if (read_reply(sock_control) == 550) { // 服务器端文件正常
                    print_reply(550);
                    close(sock_data);
                    continue;
                }
                to_client_get(sock_data, cmd.arg); // 下载文件
                print_reply(read_reply(sock_control));
                close(sock_data);
            } else if (strcmp(cmd.code, "PUSH") == 0) {
                if (to_client_put(sock_data, cmd.arg) < 0) {
                    send_response(sock_control, 553); // 文件上传失败
                } else {
                    send_response(sock_control, 200); // 文件上传成功
                }
                close(sock_data);
                print_reply(read_reply(sock_control));
            }
        }
    }
    close(sock_control);
    return 0;
}
