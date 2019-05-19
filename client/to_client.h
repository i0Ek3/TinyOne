#ifndef TO_CLIENT_H
#define TO_CLIENT_H

#include "../base/to_base.h"

int read_reply(int sock_control); // 读取服务器的回复
void print_reply(int retcode); // 打印回复消息
int to_client_read_cmd(char* buf, int size, struct command* cmd); // 读取客户端输入的命令
int to_client_get(int sock_data, char* filename); // 下载文件
int to_client_put(int sock_data, char* filename); // 上传文件
int to_client_open_conn(int sock_control); // 创建数据连接
int to_client_list(int sock_data, int sock_control); // cmd ls
int to_client_send_cmd(int sock_control, struct command* cmd); // 发送命令
int to_client_login(int sock_control); // 登录接口

#endif
