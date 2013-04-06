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
#include <fcntl.h>

void dump_packet(void const *packet, int size)
{
    struct ether_header const *eth = packet;
    printf(" Destination : %s\n", ether_ntoa((struct ether_adder const *)eth->ether_dhost));
    printf("    Source   : %s\n", ether_ntoa((struct ether_adder const *)eth->ether_shost));
    printf("     Type    : %x\n", eth->ether_type);
}

int main(int argc, char **argv)
{
    int fd, size;
    char buffer[65535];
    /* /net/eth/system  - global interface. Receive all of packets */
    /* /net/eth/eth0    - Receive packets from eth0 */
    fd = open("./net/eth/wlan0", O_RDWR);
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
        dump_packet(buffer, size);
    } while (size != 0);

    close(fd);
    return EXIT_SUCCESS;
}
