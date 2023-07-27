#ifndef __TCP_CLIENT_H__
#define __TCP_CLIENT_H__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
//#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>

void tcp_comm();
ssize_t recv_data(void *buf, size_t len);
ssize_t send_data(const void *buf, size_t len);
int tcpc_connect(void);
int tcps_accept(void);
void tcp_destroy(void);
#endif
