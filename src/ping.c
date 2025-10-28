#include "ft_ping.h"

int parse_arguments(int argc, char **argv)
{
    int i;
    
    if (argc < 2)
    {
        fprintf(stderr, "ft_ping: missing host operand\n");
        return (1);
    }

    for (i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0)
            g_ping.verbose = 1;
        else if (strcmp(argv[i], "-?") == 0 || strcmp(argv[i], "--help") == 0)
        {
            g_ping.help = 1;
            return (0);
        }
        else if (strcmp(argv[i], "-q") == 0 || strcmp(argv[i], "--quiet") == 0)
            g_ping.quiet = 1;
        else if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--numeric") == 0)
            g_ping.numeric = 1;
        else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--count") == 0)
        {
            if (i + 1 >= argc)
            {
                fprintf(stderr, "ft_ping: option requires an argument -- 'c'\n");
                return (1);
            }
            g_ping.count = atoi(argv[++i]);
            if (g_ping.count <= 0)
            {
                fprintf(stderr, "ft_ping: bad number of packets to transmit.\n");
                return (1);
            }
        }
        else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--interval") == 0)
        {
            if (i + 1 >= argc)
            {
                fprintf(stderr, "ft_ping: option requires an argument -- 'i'\n");
                return (1);
            }
            g_ping.interval = atof(argv[++i]);
            if (g_ping.interval < 0)
            {
                fprintf(stderr, "ft_ping: bad timing interval.\n");
                return (1);
            }
        }
        else if (strcmp(argv[i], "-w") == 0 || strcmp(argv[i], "--timeout") == 0)
        {
            if (i + 1 >= argc)
            {
                fprintf(stderr, "ft_ping: option requires an argument -- 'w'\n");
                return (1);
            }
            g_ping.timeout = atoi(argv[++i]);
            if (g_ping.timeout <= 0)
            {
                fprintf(stderr, "ft_ping: bad wait time.\n");
                return (1);
            }
        }
        else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--size") == 0)
        {
            if (i + 1 >= argc)
            {
                fprintf(stderr, "ft_ping: option requires an argument -- 's'\n");
                return (1);
            }
            g_ping.packet_size = atoi(argv[++i]);
            if (g_ping.packet_size < 0 || g_ping.packet_size > MAX_PACKET_SIZE - ICMP_HEADER_SIZE - 20)
            {
                fprintf(stderr, "ft_ping: packet size too large.\n");
                return (1);
            }
        }
        else if (argv[i][0] != '-')
        {
            if (g_ping.target == NULL)
                g_ping.target = argv[i];
            else
            {
                fprintf(stderr, "ft_ping: extra operand '%s'\n", argv[i]);
                return (1);
            }
        }
        else
        {
            fprintf(stderr, "ft_ping: invalid option -- '%s'\n", argv[i]);
            return (1);
        }
    }

    if (g_ping.target == NULL)
    {
        fprintf(stderr, "ft_ping: missing host operand\n");
        return (1);
    }

    if (strlen(g_ping.target) == 0)
    {
        fprintf(stderr, "ft_ping: invalid host operand\n");
        return (1);
    }

    return (0);
}

void print_usage(void)
{
    printf("Usage: ft_ping [OPTION...] HOST ...\n");
    printf("Send ICMP ECHO_REQUEST packets to network hosts.\n\n");
    printf("Options:\n");
    printf("  -c, --count=NUMBER         stop after sending NUMBER packets\n");
    printf("  -i, --interval=NUMBER      wait NUMBER seconds between sending each packet\n");
    printf("  -n, --numeric              do not resolve host addresses\n");
    printf("  -q, --quiet                quiet output\n");
    printf("  -s, --size=NUMBER          send NUMBER data octets\n");
    printf("  -v, --verbose              verbose output\n");
    printf("  -w, --timeout=N            stop after N seconds\n");
    printf("  -?, --help                 give this help list\n");
}

int resolve_hostname(const char *hostname, char *ip)
{
    struct hostent *he;
    struct sockaddr_in addr;

    if (inet_aton(hostname, &addr.sin_addr))
    {
        strcpy(ip, hostname);
        return (0);
    }

    he = gethostbyname(hostname);
    if (he == NULL)
        return (1);

    strcpy(ip, inet_ntoa(*((struct in_addr*)he->h_addr_list[0])));
    return (0);
}

int create_socket(void)
{
    g_ping.sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (g_ping.sockfd < 0)
        return 1;

    g_ping.dest_addr.sin_family = AF_INET;
    inet_aton(g_ping.target_ip, &g_ping.dest_addr.sin_addr);

    return 0;
}

int send_ping(void)
{
    struct icmphdr icmp_hdr;
    struct timeval timestamp;
    char packet[MAX_PACKET_SIZE];
    int packet_size;
    int idx;

    packet_size = g_ping.packet_size > 0 ? g_ping.packet_size + ICMP_HEADER_SIZE : DEFAULT_PACKET_SIZE;

    memset(&icmp_hdr, 0, sizeof(icmp_hdr));
    icmp_hdr.type = ICMP_ECHO;
    icmp_hdr.code = 0;
    icmp_hdr.un.echo.id = g_ping.pid & 0xFFFF;
    icmp_hdr.un.echo.sequence = g_ping.stats.transmitted;

    gettimeofday(&timestamp, NULL);

    memcpy(packet, &icmp_hdr, sizeof(icmp_hdr));
    memcpy(packet + sizeof(icmp_hdr), &timestamp, sizeof(timestamp));

    for (idx = sizeof(icmp_hdr) + sizeof(timestamp); idx < packet_size; idx++)
        packet[idx] = 'A' + (idx % 26);

    ((struct icmphdr*)packet)->checksum = 0;
    ((struct icmphdr*)packet)->checksum = checksum(packet, packet_size);

    if (sendto(g_ping.sockfd, packet, packet_size, 0, 
               (struct sockaddr*)&g_ping.dest_addr, sizeof(g_ping.dest_addr)) < 0) {
        if (g_ping.verbose)
            perror("sendto");
        return 1;
    }

    g_ping.stats.transmitted++;
    return 0;
}

int receive_ping(void)
{
    char buffer[1024];
    struct sockaddr_in recv_addr;
    socklen_t addr_len = sizeof(recv_addr);
    struct timeval tv_recv, tv_sent;
    struct iphdr *ip_hdr;
    struct icmphdr *icmp_hdr;
    double rtt;
    fd_set read_fds;
    struct timeval timeout;
    int got_reply = 0;

    while (!got_reply) {
        FD_ZERO(&read_fds);
        FD_SET(g_ping.sockfd, &read_fds);
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        if (select(g_ping.sockfd + 1, &read_fds, NULL, NULL, &timeout) <= 0) {
            return 0;
        }

        ssize_t bytes = recvfrom(g_ping.sockfd, buffer, sizeof(buffer), 0,
                                (struct sockaddr*)&recv_addr, &addr_len);
        
        gettimeofday(&tv_recv, NULL);

        if (bytes < 0) {
            if (g_ping.verbose)
                perror("recvfrom");
            return 1;
        }

        if (bytes < (ssize_t)sizeof(struct iphdr)) {
            if (g_ping.verbose)
                printf("Received packet too small for IP header\n");
            continue;
        }

        ip_hdr = (struct iphdr*)buffer;
        size_t ip_header_len = ip_hdr->ihl * 4;
        
        if (ip_header_len < sizeof(struct iphdr) || 
            bytes < (ssize_t)(ip_header_len + sizeof(struct icmphdr))) {
            if (g_ping.verbose)
                printf("Received malformed packet\n");
            continue;
        }

        icmp_hdr = (struct icmphdr*)(buffer + ip_header_len);

        if (icmp_hdr->type == ICMP_ECHOREPLY && icmp_hdr->un.echo.id == (g_ping.pid & 0xFFFF)) {
            size_t timestamp_offset = ip_header_len + sizeof(struct icmphdr);
            if (bytes < (ssize_t)(timestamp_offset + sizeof(tv_sent))) {
                if (g_ping.verbose)
                    printf("Received packet too small for timestamp\n");
                continue;
            }
            memcpy(&tv_sent, buffer + timestamp_offset, sizeof(tv_sent));
            rtt = get_time_diff(tv_sent, tv_recv);

            if (!g_ping.quiet) {
                printf("%ld bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n",
                       bytes - (ssize_t)ip_header_len, g_ping.target_ip,
                       icmp_hdr->un.echo.sequence, ip_hdr->ttl, rtt);
            }

            g_ping.stats.received++;
            calculate_stats(rtt);
            got_reply = 1;
        } else if (icmp_hdr->type == ICMP_TIME_EXCEEDED) {
            if (g_ping.verbose) {
                printf("From %s icmp_seq=%d Time to live exceeded\n",
                       inet_ntoa(recv_addr.sin_addr), g_ping.stats.transmitted);
            }
            got_reply = 1;
        } else if (icmp_hdr->type == ICMP_DEST_UNREACH) {
            if (g_ping.verbose) {
                printf("From %s icmp_seq=%d Destination Host Unreachable\n",
                       inet_ntoa(recv_addr.sin_addr), g_ping.stats.transmitted);
            }
            got_reply = 1;
        } else if (g_ping.verbose) {
            printf("Received ICMP packet: type=%d, code=%d\n", icmp_hdr->type, icmp_hdr->code);
        }
    }

    return 0;
}

void signal_handler(int sig)
{
    if (sig == SIGINT) {
        gettimeofday(&g_ping.stats.end_time, NULL);
        printf("\n--- %s ping statistics ---\n", g_ping.target);
        print_stats();
        close(g_ping.sockfd);
        exit(0);
    }
}