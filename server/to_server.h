#ifndef TO_SERVER_H
#define TO_SERVER_H

#include "../base/to_base.h"

void to_server_retrive(int sock_control, int sock_data, char* filename);
int to_server_list(int sock_data, int sock_control);
int to_server_conn(int sock_control);
int to_server_check(char* user, char* passwd);
int to_server_login(int sock_control);
int to_server_recv_cmd(int sock_control, char* cmd, char* arg);
void to_server_process(int sock_control);

#endif
