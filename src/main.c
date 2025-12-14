#include "ft_ping.h"

t_ping_config g_ping;

int main(int argc, char **argv)
{
    struct timeval current_time;
    double elapsed;
    double interval;
    
    memset(&g_ping, 0, sizeof(g_ping));
    
    /* Set default values */
    g_ping.interval = 1.0;
    
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
        fprintf(stderr, "ft_ping: unknown host\n");
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

    gettimeofday(&g_ping.stats.start_time, NULL);

    /* Calculate packet size for display */
    int data_bytes = g_ping.packet_size > 0 ? g_ping.packet_size : (DEFAULT_PACKET_SIZE - ICMP_HEADER_SIZE);
    
    if (g_ping.verbose)
        printf("PING %s (%s): %d data bytes, id 0x%04x = %u\n", 
               g_ping.target, g_ping.target_ip, data_bytes, 
               g_ping.pid & 0xFFFF, g_ping.pid & 0xFFFF);
    else
        printf("PING %s (%s): %d data bytes\n", g_ping.target, g_ping.target_ip, data_bytes);

    while (1)
    {
        /* Check timeout */
        if (g_ping.timeout > 0) {
            gettimeofday(&current_time, NULL);
            elapsed = get_time_diff(g_ping.stats.start_time, current_time) / 1000.0;
            if (elapsed >= g_ping.timeout) {
                break;
            }
        }
        
        /* Check count */
        if (g_ping.count > 0 && g_ping.stats.transmitted >= g_ping.count) {
            break;
        }
        
        if (send_ping() != 0)
            break;
        if (receive_ping() != 0)
            break;
            
        /* Wait interval before next ping */
        interval = g_ping.interval > 0 ? g_ping.interval : 1.0;
        if (g_ping.count > 0 && g_ping.stats.transmitted >= g_ping.count)
            break;
        usleep((useconds_t)(interval * 1000000));
    }

    gettimeofday(&g_ping.stats.end_time, NULL);
    printf("--- %s ping statistics ---\n", g_ping.target);
    print_stats();
    close(g_ping.sockfd);
    return (0);
}