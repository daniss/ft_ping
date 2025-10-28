#ifndef FT_PING_H
#define FT_PING_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/select.h>
#include <math.h>

#define DEFAULT_PACKET_SIZE 64
#define ICMP_HEADER_SIZE 8
#define MAX_PACKET_SIZE 65535

typedef struct s_ping_stats {
    int transmitted;
    int received;
    int lost;
    double min_time;
    double max_time;
    double total_time;
    double avg_time;
    double sum_squares;
    struct timeval start_time;
    struct timeval end_time;
} t_ping_stats;

typedef struct s_ping_config {
    char *target;
    char target_ip[INET_ADDRSTRLEN];
    int verbose;
    int help;
    int quiet;
    int numeric;
    int count;
    double interval;
    int timeout;
    int packet_size;
    int sockfd;
    struct sockaddr_in dest_addr;
    t_ping_stats stats;
    pid_t pid;
} t_ping_config;

extern t_ping_config g_ping;

void print_usage(void);
int parse_arguments(int argc, char **argv);
int resolve_hostname(const char *hostname, char *ip);
int create_socket(void);
int send_ping(void);
int receive_ping(void);
void calculate_stats(double rtt);
void print_stats(void);
void signal_handler(int sig);
unsigned short checksum(void *b, int len);
double get_time_diff(struct timeval start, struct timeval end);

#endif