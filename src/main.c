#include "ft_ping.h"

t_ping_config g_ping;

int main(int argc, char **argv)
{
    memset(&g_ping, 0, sizeof(g_ping));
    
    if (parse_arguments(argc, argv) != 0)
        return (1);

    if (g_ping.help)
    {
        print_usage();
        return (0);
    }

    if (geteuid() != 0)
    {
        fprintf(stderr, "ft_ping: Operation not permitted\n");
        return (1);
    }

    if (resolve_hostname(g_ping.target, g_ping.target_ip) != 0)
    {
        fprintf(stderr, "ping: cannot resolve %s: Unknown host\n", g_ping.target);
        return (1);
    }

    if (create_socket() != 0)
    {
        fprintf(stderr, "ft_ping: socket creation failed\n");
        return (1);
    }

    g_ping.pid = getpid();
    g_ping.stats.min_time = 999999.0; /* initialize to high value */

    signal(SIGINT, signal_handler);

    printf("PING %s (%s): %d data bytes\n", g_ping.target, g_ping.target_ip, PACKET_SIZE - ICMP_HEADER_SIZE);

    while (1)
    {
        if (send_ping() != 0)
            break;
        if (receive_ping() != 0)
            break;
        sleep(1);
    }

    close(g_ping.sockfd);
    return (0);
}