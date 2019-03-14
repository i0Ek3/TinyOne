#ifndef TO_CLIENT_H
#define TO_CLIENT_H

#include "../base/to_base.h"

int read_reply();
void print_reply(int retcode);
int to_client_read_cmd(char* buf, int size, struct command* cmd); // conflict with send_cmd
int to_client_get(int sock_data, int sock_control, char* arg);
int to_client_put(int sock_data, char* filename);
int to_client_open_conn(int sock_con);
int to_client_list(int sock_data, int sock_conn);
int to_client_send_cmd(struct command* cmd);
void to_client_login();

#endif
