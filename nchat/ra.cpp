#include <cstdlib>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>

// #include <cstdio>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <sys/epoll.h>

#include "chat.h"
#include "common.h"
#include "announce_message.h"
#include "chat_message.h"
#include "ra.h"

ra::ra(const uint16_t udp_port, const uint16_t tcp_port)
    : R(tcp_port), A(udp_port)
{
    get_mac(mac_addr);
    std::cout << "host mac = ";
    print_mac(mac_addr);
    std::cout << std::endl;
    epollfd = epoll_create1(0);
    if (epollfd < 0)
    {
        die("sl::sl : epoll_create1");
    }
    epoll_event ev;
    ev.events = EPOLLIN | EPOLLOUT | EPOLLERR;
    ev.data.fd = R.sock;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, R.sock, &ev) == -1)
    {
        die("sl::sl : epoll_ctl add R.sock");
    }
}

ra::~ra()
{
}

void ra::start()
{
    FD_ZERO(&fds);
    FD_SET(R.sock, &fds);
    maxfd = R.sock;
}

const size_t MAX_EVENTS = 20;

void ra::cycle()
{
    A.announce();
    epoll_event events[MAX_EVENTS];
    int cnt = epoll_wait(epollfd, events, MAX_EVENTS, 1);
    if (cnt < 0)
    {
        die("ra::cycle : epoll_wait");
    }
    for (int i = 0; i < cnt; ++i)
    {
        if (events[i].data.fd == R.sock)
        {
            receive_cm();
        }
    }
}

void ra::receive_cm()
{
    chat_message msg = R.receive_message();
    print_mac(msg.mac_addr);
    std::cout << " : (" << msg.len << ") " << msg.text << std::endl;
}
