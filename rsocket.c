#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <math.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
// #include <signal.h>
#include <errno.h>
// #include <time.h>

#include "rsocket.h"

void* r_thread_runner(void*);
void* s_thread_runner(void*);
pthread_mutex_t mutex1;
int udp_sockfd;
msg_sent_info* unack_table;
msg_recv* recv_table;
int message_seq = 0;

int r_socket(int domain, int type, int protocol)
{
  if (type != SOCK_MRP)
  {
    errno = ESOCKTNOSUPPORT;
    return -1;
  }
  udp_sockfd = socket(domain, SOCK_DGRAM, protocol);
  if (udp_sockfd < 0) return udp_sockfd;
  unack_table = (msg_sent_info*)malloc(TABLE_SIZE * sizeof(msg_sent_info));
  recv_table = (msg_recv*)malloc(TABLE_SIZE * sizeof(msg_recv));
  // initialisation of tables
  for (int i = 0; i < TABLE_SIZE; i++)
  {
    unack_table[i].dest_port = -1;
    memset(unack_table[i].message, '\0', TABLE_SIZE);
    unack_table[i].time_sent = -1;
  }
  for (int i = 0; i < TABLE_SIZE; i++)
  {
    memset(recv_table[i].message, '\0', TABLE_SIZE);
    recv_table[i].time = -1;
  }
  // preparing argument
  // msg_info arg;
  // arg.recv_table = recv_table;
  // arg.unack_table = unack_table;
  // creating threads
  pthread_mutex_init(&mutex1, NULL);
  pthread_t R_thread, S_thread;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_create(&R_thread, &attr, r_thread_runner, NULL);
  pthread_create(&S_thread, &attr, r_thread_runner,NULL);
  return udp_sockfd;
}

int r_bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen)
{
  int bind_ret = bind(sockfd, addr, addrlen);
  return bind_ret;
}
ssize_t r_sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen)
{
  // send message with sequence number and stores appr info
  char buffer[MESG_SIZE];
  for (int i = 0; i < MESG_SIZE; i++) buffer[i] = '\0';
  sprintf(buffer, "%d", message_seq);
  strcat(buffer, buf);
  buffer[strlen(buffer)] = '\0';
  pthread_mutex_lock(&mutex1);
  message_seq++;
  for (int i = 0; i < TABLE_SIZE; i++)
  {
    if (unack_table[i].dest_port == -1)
    {
      unack_table[i].dest_addr = ((struct sockaddr_in*)dest_addr) -> sin_addr;
      unack_table[i].dest_port = ((struct sockaddr_in*)dest_addr) -> sin_port;
      strcpy(unack_table[i].message, buffer);
      unack_table[i].time_sent = time(NULL);
      break;
    }
  }
  int send_ret = sendto(sockfd, buffer, strlen(buffer) + 1, 0, dest_addr, addrlen);
  pthread_mutex_unlock(&mutex1);
  return send_ret;
}
ssize_t r_recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen)
{
  // check received-message table and block if no message
  time_t min_time = -1;
  int min_time_index = 0;
  while (min_time == -1)
  {
    for (int i = 0; i < TABLE_SIZE; i++)
    {
      if (recv_table[i].time != -1)
      {
        if (min_time == -1 || min_time > recv_table[i].time)
        {
          min_time = recv_table[i].time;
          min_time_index = i;
        }
      }
    }
    sleep(SLEEP_TIME);
  }
  strcpy(buf, recv_table[min_time_index].message);
  size_t size = sizeof(recv_table[min_time_index].message);
  recv_table[min_time_index].time = -1;
  memset(recv_table[min_time_index].message, '\0', TABLE_SIZE);
  return size;
}
int r_close(int fd)
{
  return close(fd);
}
int dropMessage(float p)
{
  return -1;
}
void* r_thread_runner(void* param)
{
  // msg_info* arg = (msg_info*) param;
  pthread_mutex_lock(&mutex1);
// wait for recvfrom and check for messages and acks
  // wait for recvfrom
  int recv_bytes;
  char buffer[MESG_SIZE];
  struct sockaddr_in cli_addr;
  socklen_t cli_len = sizeof(cli_addr);
  recv_bytes = recvfrom(udp_sockfd, buffer, MESG_SIZE, 0, (struct sockaddr*)&cli_addr, &cli_len);
  if (recv_bytes < 0) return recv_bytes;
  // check message
  // acknowledgment is sent in form ACK[message_seq]
  if (strlen(buffer) != 0 && buffer[0] == 'A')
  {
    // message is ACK
  }else
  {
    // message is data
  }
  pthread_mutex_unlock(&mutex1);
}

void* s_thread_runner(void* param)
{
  while(1)
  {
    sleep(T);
    pthread_mutex_lock(&mutex1);
    // see if timeout period is over
    for (int i = 0; i < TABLE_SIZE; i++)
    {
      if (unack_table[i].dest_port != -1 && time(NULL) - unack_table[i].time_sent > 2 * T)
      {
        // see if timeout period is over and resend and reset
        struct sockaddr_in cli_addr;
        cli_addr.sin_addr = unack_table[i].dest_addr;
        cli_addr.sin_family = AF_INET;
        cli_addr.sin_port = unack_table[i].dest_port;
        int cli_len = sizeof(cli_addr);
        sendto(udp_sockfd, unack_table[i].message, strlen(unack_table[i].message) + 1, 0, (struct sockaddr*)&cli_addr, cli_len);
        unack_table[i].time_sent = time(NULL);
      }
    }
    pthread_mutex_unlock(&mutex1);
  }
}
void itoa(int value, char* result) {
    char* ptr = result, *ptr1 = result, tmp_char;
    int tmp_value;
    char int_map[] = "0123456789";
    do
    {
      tmp_value = value;
      value /= 10;
      *ptr = int_map[(tmp_value - value * 10)];
      ptr++;
    } while ( value );

    // Apply negative sign
    if (tmp_value < 0) *ptr++ = '-';
    *ptr-- = '\0';
    while(ptr1 < ptr)
    {
      tmp_char = *ptr;
      *ptr--= *ptr1;
      *ptr1++ = tmp_char;
    }
}
// int atoi(char* buffer, int len)
// {
//   int ret = 0;
//   for (int i = 0; i < len; i++)
//   {
//     printf("%d\n", buffer[i]);
//     ret = 10 * ret + buffer[i];
//   }
//   return ret;
// }
