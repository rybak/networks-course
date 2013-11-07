#include <cstdint>
#include <cstdio>

#include <ctype.h>
#include <cstdlib>


#include "common.h"
#include "announcer.h"
#include "chat.h"
#include "ma.h"
#include "kbhit.h"

#include "chatter.h"
#include "ports.h"

using namespace std;

void print_usage(char *cmd)
{
    printf("Usage:\n");
    printf("\t%s \n", cmd);
}

const int MSG_KEY = 's';
const int QUIT_KEY = 'q';

void quit()
{
    printf("Quiting ... \n");
    sleep(0);
    exit(0);
}

int main(int argc, char *argv[])
{
    char *tcp_port_val = NULL;
    char *udp_port_val = NULL;
    int index;
    int optch;

    opterr = 0;

    while ((optch = getopt (argc, argv, "t:u:")) != -1)
    {
        switch (optch)
        {
            case 't':
                tcp_port_val = optarg;
                break;
            case 'u':
                udp_port_val = optarg;
                break;
            case '?':
                if (optopt == 't' || optopt == 'u')
                    fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                else if (isprint (optopt))
                    fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf (stderr,
                            "Unknown option character `\\x%x'.\n",
                            optopt);
                return 1;
            default:
                abort ();
        }
    }
    for (index = optind; index < argc; index++)
    {
        printf ("Non-option argument %s\n", argv[index]);
    }
    uint16_t tcp_port = tcp_port_val != NULL ? atoi(tcp_port_val)
        : TCP_PORT;

    mac_addr_t ma;
    get_mac(ma);
    print_mac(ma);
    announcer a;
    chatter c;
    printf("Send key = '%c'\n", MSG_KEY);
    timespec sleep_time;
    sleep_time.tv_sec = 0;
    sleep_time.tv_nsec = 100l * 1000l * 1000l;
    for(;;)
    {
        a.announce();
        nanosleep(&sleep_time, NULL);
        if (kbhit())
        {
            int ch = getchar();
            printf("\rKey %d = '%c' is pressed\n", (int) ch, ch);
            switch(ch)
            {
                case MSG_KEY:
                    c.read_message();
                    break;
                case QUIT_KEY:
                    quit();
                    break;
                default:
                    break;
            }
        }
        c.cycle();
    } 
}

