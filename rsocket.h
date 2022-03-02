#ifndef _RSOCKET_H_
#define _RSOCKET_H_


#include <sys/socket.h>
#include <sys/types.h>

#define SOCK_MRP 99
#define MESG_SIZE 100
#define TABLE_SIZE 50

typedef struct
{
  char message[MESG_SIZE];
  float time_sent;
}msg_sent_info;
typedef struct
{
  char message[MESG_SIZE];
}msg_recv;

int r_socket(int, int, int);
int r_bind(int, const struct sockaddr*, socklen_t);
ssize_t r_sendto(int, const void *, size_t, int, const struct sockaddr*, socklen_t);
ssize_t r_recvfrom(int, void*, size_t, int, struct sockaddr*, socklen_t*);
int close(int);
int dropMessage(float);

#endif