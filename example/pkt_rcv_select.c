/*
 *  Polling program
 *   explaining the way to use planetfs
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <net/ethernet.h>
#include <netinet/ether.h>
#include <arpa/inet.h>
#include <limits.h>
#include <fcntl.h>

#define MAX(a, b) ((a) < (b) ? (b) : (a))

void print_ethernet_header(struct ether_header const *eth, const char ifname[])
{
    printf("%s: %17s >> ", ifname, ether_ntoa((struct ether_addr *)eth->ether_shost));
    printf("%17s ", ether_ntoa((struct ether_addr *)eth->ether_dhost));
    printf("type=%x\n", ntohs(eth->ether_type));
}

int monitor_once(int iffd, const char ifname[])
{
    static char buffer[65535];
    int size = read(iffd, buffer, sizeof (buffer));
    if (size <= 0)
        perror("read");
    else
        print_ethernet_header((struct ether_header *)buffer, ifname);
    return size;
}

int packet_capture(
    int iffd1, int iffd2, const char ifname1[], const char ifname2[]
)
{
    fd_set rfds, backupfds;
    FD_ZERO(&backupfds);
    FD_SET(iffd1, &backupfds);
    FD_SET(iffd2, &backupfds);

    while (1) {
        rfds = backupfds;
        if (select(MAX(iffd1, iffd2) + 1, &rfds, NULL, NULL, NULL) < 0) {
            perror("select");
            return -1;
        }
        if (FD_ISSET(iffd1, &rfds))
            monitor_once(iffd1, ifname1);
        if (FD_ISSET(iffd2, &rfds))
            monitor_once(iffd2, ifname2);
    }
}

int main(int argc, char **argv)
{
    int iffd1, iffd2;
    char ifpath1[PATH_MAX], ifpath2[PATH_MAX];

    if (argc < 3) {
        printf("Usage: %s IFNAME1 IFNAME2\n", argv[0]);
        return 1;
    }

    snprintf(ifpath1, sizeof ifpath1, "/net/eth/%s", argv[1]);
    snprintf(ifpath2, sizeof ifpath2, "/net/eth/%s", argv[2]);
    iffd1 = open(ifpath1, O_RDONLY);
    iffd2 = open(ifpath2, O_RDONLY);
    if (iffd1 < 0 || iffd2 < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    packet_capture(iffd1, iffd2, argv[1], argv[2]);

    close(iffd1);
    close(iffd2);
    return 0;
}
