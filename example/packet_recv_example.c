/*
 *  Packet socket sample program
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

void print_ethernet_header(struct ether_header const *eth)
{
    printf("%17s >> ", ether_ntoa((struct ether_addr *)eth->ether_shost));
    printf("%17s ", ether_ntoa((struct ether_addr *)eth->ether_dhost));
    printf("type=%x\n", ntohs(eth->ether_type));
}

int main(int argc, char **argv)
{
    int fd, size;
    char path[PATH_MAX] = "/net/eth/", buffer[65535];

    if (argc < 2) {
        printf("Usage: %s INTERFACE_NAME\n", argv[0]);
        return 1;
    }

    /* /net/eth/eth0 - Receive packets from eth0 */
    strcat(path, argv[1]);
    fd = open(path, O_RDWR);
    if (fd < 0) {
        perror("open");
        return EXIT_FAILURE;
    }

    /* Receive ethernet frames */
    do {
        size = read(fd, buffer, sizeof (buffer));
        if (size < 0) {
            perror("read");
            break;
        }
        /* Display ethernet header */
        print_ethernet_header((struct ether_header *)buffer);
    } while (size != 0);

    close(fd);
    return EXIT_SUCCESS;
}
