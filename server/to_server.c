#include "to_server.h"


void to_server_retrive(int sock_control, int sock_data, char* filename) { // 服务器端下载命令的实现
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

int to_server_list(int sock_data, int sock_control) { // list 命令的实现
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

    fseek(fd, SEEK_SET, 0); // 文件描述符偏移
    send_response(sock_control, 1); // 准备发送数据
    memset(data, 0, MAXSIZE);

    while ((read_size = fread(data, 1, MAXSIZE, fd)) > 0) {
        if (send(sock_data, data, read_size, 0) < 0) {
            perror("Send data error!");
            memset(data, 0, MAXSIZE);
        }
    }

    fclose(fd);
    send_response(sock_control, 226);
    return 0;
}

void to_server_push(int sock_control, int sock_data, char* filename) { // upload
    int ack;
    //int sock_control = 0;
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

    char name[260];
    memset(name, 0, sizeof(name));
    strcpy(name, "ftp://"); // 复制字符串到name中
    strcat(name, filename); // 连接给定的两个字符串
    int fd = open(name, O_CREAT|O_WRONLY, 0644);
    if (fd < 0) {
        send_response(sock_control, 502); // 命令执行失败
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
    getpeername(sock_control, (struct sockaddr*)&client_addr, &len); // 获取预sockfd连接的对端的地址信息，结果存在peeraddr所指向的空间中
    inet_ntop(AF_INET, &client_addr.sin_addr, buf, sizeof(buf)); // 转换ipv4地址

    if ((sock_data = socket_connect(buf, PORT)) < 0) {
        return -1;
    }

    return sock_data;
}

int to_server_check(const char* user, const char* passwd) { // 登录检查
    char username[MAXSIZE];
    char password[MAXSIZE];
    char buf[MAXSIZE];
    char* line = NULL;
    size_t read_size;
    size_t len = 0;
    FILE* fd = fopen("auth.txt", "r");
    int auth = -1;

    if (NULL == fd) {
        perror("404 File Not Found!");
        exit(1);
    }

    while ((read_size = getline(&line, &len, fd)) != -1) {
        memset(buf, 0, MAXSIZE);
        strcpy(buf, line);

        char *getuser = strtok(buf, " "); // 切割字符串，分离出用户名
        strcpy(username, getuser);

        if (getuser != NULL) {
            char *getpwd = strtok(NULL, " "); // 分离出密码
            strcpy(password, getpwd);
        }

        trim_char(password, (int)strlen(password)); // 去掉字符串中的空格和换行

        // 用户名和密码验证成功
        if ((strcmp(user, username) == 0) && (strcmp(passwd, password)) == 0) {
            auth = 1;
            break;
        }
    }

    free(line);
    fclose(fd);
    return auth;
}

int to_server_login(int sock_control) { // 登录接口
    char buf[MAXSIZE];
    char user[MAXSIZE];
    char passwd[MAXSIZE];
    memset(buf, 0, MAXSIZE);
    memset(user, 0, MAXSIZE);
    memset(passwd, 0, MAXSIZE);

    // 从文件描述符中读取数据
    if ((recv_data(sock_control, buf, sizeof(buf))) == -1) {
        perror("Failed to receive data!\n");
        exit(1);
    }

    int i = 5;
    int n = 0;
    while (buf[i] != 0) { // 保存用户名
        user[n++] = buf[i++];
    }

    send_response(sock_control, 331); // 通知用户输入密码
    memset(buf, 0, MAXSIZE);

    // 获取密码
    if ((recv_data(sock_control, buf, sizeof(buf))) == -1) {
        perror("Failed to receive data!\n");
        exit(1);
    }

    i = 5;
    n = 0;
    while (buf[i] != 0) {
        passwd[n++] = buf[i++];
    }

    return (to_server_check(user, passwd)); // 验证登录
}

int to_server_recv_cmd(int sock_control, char* cmd, char* arg) { // 接收来自客户端发送的命令
    int retcode = 200;
    char buf[MAXSIZE];

    // 初始化命令及其参数
    memset(buf, 0, MAXSIZE);
    memset(cmd, 0, 5);
    memset(arg, 0, MAXSIZE);

    // 从文件描述符中读数据
    if ((recv_data(sock_control, buf, sizeof(buf))) == -1) {
        perror("Failed to receive data!\n");
        return -1;
    }

    strncpy(cmd, buf, 4);
    char* tmp = buf + 5;
    strcpy(arg, tmp);

    if (strcmp(cmd, "QUIT") == 0) {
        retcode = 221; // 退出登录
    } else if (strcmp(cmd, "USER") == 0 || (strcmp(cmd, "PASS")) == 0 || (strcmp(cmd, "LIST")) == 0 || (strcmp(cmd, "RETR")) == 0 || (strcmp(cmd, "PUSH")) == 0) {
        retcode = 200; // 命令执行成功
    } else {
        retcode = 502; // 命令未执行
    }

    send_response(sock_control, retcode); // 发送状态到文件描述符中
    return retcode;
}

void to_server_process(int sock_control) { // 处理ftp事件
    int sock_data;
    char cmd[5];
    char arg[MAXSIZE];

    send_response(sock_control, 220);  // 发送相应码，服务就绪
    if (to_server_login(sock_control) == 1) { // 登录成功
        send_response(sock_control, 230); // auth is ok
    } else { // 登录失败
        send_response(sock_control, 430); // auth is failed
        exit(0);
    }

    while (1) {
        // 解析命令
        int retcode = to_server_recv_cmd(sock_control, cmd, arg);
        if ((retcode < 0) || (retcode == 221)) { // 出错或者quit
            break;
        }

        if (retcode == 200) { // 开始处理事件
            if ((sock_data = to_server_conn(sock_control)) < 0) {
                close(sock_control);
                exit(1);
            }

            // 命令执行
            if (strcmp(cmd, "LIST") == 0) {
                to_server_list(sock_data, sock_control);
            } else if (strcmp(cmd, "RETR") == 0) {
                to_server_retrive(sock_control, sock_data, arg);
            } else if (strcmp(cmd, "PUSH") == 0) {
                to_server_push(sock_control, sock_data, arg);
            }
            close(sock_data);
        }
    }
}

int main(int argc, char* argv[])
{
    if (argc != 2) {
        perror("Usage: ./tos port\n");
        exit(0);
    }

    int port = atoi(argv[1]);

    int listenfd;  // 创建监听器
    if ((listenfd = socket_create(port)) < 0) {
        perror("Failed to create socket!");
        exit(1);
    }

    int sock_control;
    int pid;
    for (;;) { // 循环接受请求
        if ((sock_control = socket_accept(listenfd)) < 0) {
            break;
        }

        if ((pid = fork()) < 0) { 
            perror("Failed to fork child process!");
        } else if (pid == 0) { // 创建子进程
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
