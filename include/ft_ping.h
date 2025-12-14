#ifndef FT_PING_H
#define FT_PING_H

#define _DEFAULT_SOURCE
#define _BSD_SOURCE

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
#include <signal.h>
#include <sys/time.h>
#include <math.h>

#define DEFAULT_PACKET_SIZE 64
#define ICMP_HEADER_SIZE 8
#define MAX_PACKET_SIZE 65535

typedef struct s_stats {
    int transmitted;
    int received;
    double min_time;
    double max_time;
    double avg_time;
    double total_time;
    double sum_squares;
    struct timeval start_time;
    struct timeval end_time;
} t_stats;

typedef struct s_ping_config {
    char *target;
    char target_ip[INET_ADDRSTRLEN];
    int sockfd;
    struct sockaddr_in dest_addr;
    pid_t pid;
    int verbose;
    int help;
    int count;
    int timeout;
    int packet_size;
    double interval;
    t_stats stats;
} t_ping_config;

extern t_ping_config g_ping;

int parse_arguments(int argc, char **argv);
void print_usage(void);
int resolve_hostname(const char *hostname, char *ip);
int create_socket(void);
int send_ping(void);
int receive_ping(void);
void signal_handler(int sig);

unsigned short checksum(void *b, int len);
double get_time_diff(struct timeval start, struct timeval end);
void calculate_stats(double rtt);
void print_stats(void);

#endif