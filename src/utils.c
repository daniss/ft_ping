#include "ft_ping.h"

unsigned short checksum(void *b, int len)
{
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    while (len > 1) {
        sum += *buf++;
        len -= 2;
    }

    if (len == 1)
        sum += *(unsigned char*)buf << 8;

    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);

    result = ~sum;
    return result;
}

double get_time_diff(struct timeval start, struct timeval end)
{
    return (double)(end.tv_sec - start.tv_sec) * 1000.0 +
           (double)(end.tv_usec - start.tv_usec) / 1000.0;
}

void calculate_stats(double rtt)
{
    if (rtt < g_ping.stats.min_time)
        g_ping.stats.min_time = rtt;
    if (rtt > g_ping.stats.max_time)
        g_ping.stats.max_time = rtt;
    g_ping.stats.total_time += rtt;
    g_ping.stats.sum_squares += rtt * rtt;
    g_ping.stats.avg_time = g_ping.stats.total_time / g_ping.stats.received;
}

void print_stats(void)
{
    int lost = g_ping.stats.transmitted - g_ping.stats.received;
    double loss_percent = 0.0;
    
    if (g_ping.stats.transmitted > 0)
        loss_percent = (double)lost / g_ping.stats.transmitted * 100.0;

    printf("%d packets transmitted, %d packets received, %.0f%% packet loss\n",
           g_ping.stats.transmitted, g_ping.stats.received, loss_percent);

    if (g_ping.stats.received > 0) {
        double variance = (g_ping.stats.sum_squares / g_ping.stats.received) - 
                         (g_ping.stats.avg_time * g_ping.stats.avg_time);
        double stddev = sqrt(variance > 0 ? variance : 0);
        printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n",
               g_ping.stats.min_time, g_ping.stats.avg_time, g_ping.stats.max_time, stddev);
    }
}