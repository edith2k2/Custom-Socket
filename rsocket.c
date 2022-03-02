#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>

#include "rsocket.h"

int r_socket(int domain, int type, int protocol)
{
  if (type != SOCK_MRP)
  {
    errno = ESOCKTNOSUPPORT;
    return -1;
  }
  int udp_sockfd;
  udp_sockfd = socket(domain, SOCK_DGRAM, protocol);
  if (udp_sockfd < 0) return udp_sockfd;
  msg_sent_info* unack_table = (msg_sent_info*)malloc(TABLE_SIZE * sizeof(msg_sent_info));
  msg_recv* ack_table = (msg_recv*)malloc(TABLE_SIZE * sizeof(msg_recv));
  pthread_t R_thread, S_thread;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_create(&R_thread, &attr, NULL, NULL);
  pthread_create(&S_thread, &attr, NULL, NULL);
}

int r_bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen)
{

}
ssize_t r_sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen)
{

}
ssize_t r_recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen)
{

}
int close(int fd)
{

}
int dropMessage(float p)
{

}
