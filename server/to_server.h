#ifndef TO_SERVER_H
#define TO_SERVER_H

#include "../base/to_base.h"

void to_server_retrive(int sock_control, int sock_data, char* filename); // 处理文件的下载
void to_server_push(int sock_control, int sock_data, char* filename); // 处理文件的上传
int to_server_list(int sock_data, int sock_control); // cmd ls
int to_server_conn(int sock_control); // 创建一个数据连接
int to_server_check(const char* user, const char* passwd); // 登录检查
int to_server_login(int sock_control); // 登录接口
int to_server_recv_cmd(int sock_control, char* cmd, char* arg); // 从控制连接中接收命令
void to_server_process(int sock_control); // 处理一个ftp事件

#endif
