#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <iostream>
#include <algorithm>

#include <ifaddrs.h>
#include <time.h>


#include "common.h"

void die(const char *s)
{
    perror(s);
    exit(1);
}

void dontdie(const char *s)
{
    perror(s);
    printf("\n");
}

void make_udp_socket(int &fd, struct sockaddr_in &sock, uint16_t port)
{
    fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (fd < 0)
    {
        die("socket");
    }

    socklen_t si_len = sizeof(sock);
    memset(&sock, 0, si_len);
    sock.sin_family = AF_INET;
    sock.sin_port = htons(port);
    sock.sin_addr.s_addr = htonl(INADDR_ANY);

    int status = bind(fd, (struct sockaddr *) &sock, si_len);
    if (status < 0)
    {
        die("bind");
    }
}