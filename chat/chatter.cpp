#include <cstdlib>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>

#include <cstdio>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>

#include "chat.h"
#include "common.h"
#include "announce_message.h"
#include "chat_message.h"
#include "user.h"
#include "chatter.h"

chatter::chatter(const uint16_t udp_port, const uint16_t tcp_port)
    : S(tcp_port), R(tcp_port),
    A(udp_port), L(udp_port)
{
    get_mac(mac_addr);
    print_mac(mac_addr);
    printf("\n");
}

chatter::~chatter()
{
}


void chatter::start()
{
    FD_ZERO(&fds);
    FD_SET(L.sock, &fds);
    FD_SET(R.sock, &fds);
    maxfd = R.sock;
}

void chatter::cycle()
{
    A.announce();
    // erase_dead_users();
    fd_set read_fds;
    FD_ZERO(&read_fds);
    memcpy(&read_fds, &fds, sizeof(fds));
    timeval tv = {0, 100000};
    int nready = select(maxfd + 1, &read_fds, NULL, NULL, &tv);
    if (nready > 0)
    {
        printf("nready = %d\n", nready);
    }
    if (nready == -1)
    {
        die("chatter::cycle : select");
    }
    for(int i = 0; i <= maxfd && nready > 0; i++)
    {
        if (FD_ISSET(i, &read_fds))
        {
            nready--;
            if (i == L.sock)
            {
                receive_am();
            }
            else
            {
                if (i == R.sock)
                {
                    printf("new chat_message\n");
                    receive_cm();
                }
            }
        }
    }
}

void chatter::receive_am()
{
    announce_message msg = L.receive_message();
    long long id = msg.mac_addr.id;
    if (users.count(id) > 0)
    {
        users.at(id).timestamp = msg.timestamp;
        users.at(id).update();
    } else
    {
        printf("New user. MAC = ");
        print_mac(msg.mac_addr);
        printf(" ip = %d\n", msg.ip);
        users.emplace(id, user(msg));
    }
}

void chatter::receive_cm()
{
    chat_message msg = R.receive_message();
    print_mac(msg.mac_addr);
    std::cout << " : " << msg.text << std::endl;
}

namespace
{
    std::string read_message()
    {
        printf("read_message\n");
        char *msg = new char[MSG_MAX_LEN];
        printf("Enter message :\n");
        int cnt = scanf("%s", msg);
        if (cnt < 0)
        {
            perror("scanf gone wrong");
            return std::string("");
        }
        char ch = getchar();
        return std::string(msg);
    }
}
void chatter::send_message()
{
    long long ht = host_time();
    std::string text = read_message();
    if (text.length() == 0)
    {
        return;
    }
    for (auto it = users.begin(); it != users.end(); ++it)
    {
        printf("sending to ");
        print_mac(it->second.mac_addr);
        printf("\n");
        if (it->second.mac_addr.id != mac_addr.id)
        {
            S.send_message(it->second, text);
        }
    }
}


namespace
{

    void print_header()
    {
        printf("Current time : %ld\n", time(NULL));
        printf("%10s%18s%10s\n", "time", "mac", "nick");
    }

    void print_entry(const user &u)
    {
        printf("%10lld ", u.timestamp);
        print_mac(u.mac_addr);
        printf("%.10s", u.nickname.c_str());
        printf(" %d ", u.ip);
        printf("\n");
    }
}

void chatter::print_users()
{
    print_header();
    if (users.empty())
    {
        printf("(NO users)\n");
    } else
    {
        for (auto it = users.begin(); it != users.end(); ++it)
        {
            print_entry(it->second);
        }
    }
    std::cout << std::endl;
}

bool is_dead_user(const user &u)
{
    long long ht = host_time();
    return (ht - u.timestamp) / 1000l > TIME_GAP;
}

void chatter::erase_dead_users()
{
    std::vector<long long> to_erase;
    for (auto it = users.begin();
            it != users.end(); ++it)
    {
        if (is_dead_user(it->second))
        {
            to_erase.push_back(it->first);
        }
    }
    for (auto it = to_erase.begin();
            it != to_erase.end(); ++it)
    {
        users.erase(*it);
    }
}


