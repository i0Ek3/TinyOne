#include "to_base.h"

int socket_create(int port) { // 创建监听套接字
    int sockfd;
    int flag = 1;
    struct sockaddr_in sock_addr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("Failed to create socket!");
        return -1;
    }

    sock_addr.sin_family = AF_INET; // ipv4, or use AF_INET6 to enable ipv6
    sock_addr.sin_port = htons(port);
    sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) == -1) {
        // setsockopt() means enhance basic socket function
        // SOL_SOCKET is basic socket port
        // SOL_REUSEADDR is a option which means to banned TIME_WAIT to enable port force

        close(sockfd);
        perror("Failed to setsockopt!");
        return -1;
    }

    if (bind(sockfd, (struct sockaddr*)&sock_addr, sizeof(sock_addr)) < 0) {
        close(sockfd);
        perror("Failed to bind!");
        return -1;
    }

    if (listen(sockfd, BACKLOG) < 0) {
        close(sockfd);
        perror("Failed to listen!");
        return -1;
    }

    return sockfd;
}

int socket_accept(int listenfd) { // 接受连接
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    int sockfd = accept(listenfd, (struct sockaddr*)&client_addr, &len);

    if (sockfd < 0) {
        perror("Failed to accept!");
        return -1;
    }

    return sockfd;
}

int socket_connect(const char* host, const int port) { // 连接指定主机
    int sockfd;
    struct sockaddr_in server_addr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("Failed to create socket!");
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(host);

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Failed to connect server!");
        return -1;
    }

    return sockfd;
}

int recv_data(int sockfd, char* buf, int bufsize) { // 从文件描述符中读取数据
    size_t size;
    memset(buf, 0, bufsize);

    size = recv(sockfd, buf, bufsize, 0);
    if (size <= 0) {
        return -1;
    }

    return size;
}

int send_response(int sockfd, int retcode) { // 发送响应码到文件描述符
    int ret = htonl(retcode);

    if (send(sockfd, &ret, sizeof(ret), 0) < 0) {
        perror("Failed to send data!");
        return -1;
    }

    return 0;
}

void read_input(char* buf, int size) { // 从标准输入中读取一行
    char* container = NULL;
    memset(buf, 0, size);

    if (fgets(buf, size, stdin) != NULL) {
        container = strchr(buf, '\n');
        if (container != 0) {
            *container = '\0';
        }
    }
}

void trim_char(char* str, int n) { // 去除字符串中的空格和换行
    for (int i = 0; i < n; i++) {
        if (isspace(str[i])) {
            str[i] = 0;
        }

        if (str[i] == '\n') {
            str[i] = 0;
        }
    }
}
