/*
 *  DNS name resolving sample program
 *   explaining the way to use planetfs
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
    int fd, size;
    char buffer[1024], request[1024];

    if (argc < 2) {
        printf("Usage: %s HOSTNAME\n", argv[0]);
        return EXIT_FAILURE;
    }

    fd = open("/net/dns", O_RDWR);
    if (fd < 0) {
        perror("open");
        return EXIT_FAILURE;
    }

    /* 
     * Request DNS service. format:
     * 'resolve hostname'       - AF_UNSPEC
     * 'resolve_inet hostname'  - AF_INET
     * 'resolve_inet6 hostname' - AF_INET6
     */
    snprintf(request, sizeof (request), "resolve %s", argv[1]);
    size = write(fd, request, strlen(request));
    if (size < 0) {
        perror("write");
        return EXIT_FAILURE;
    }

    /* Read /net/dns to get the result */
    for (;;) {
        size = read(fd, buffer, sizeof (buffer));
        if (size <= 0)  // read error or EOF
            break;
        /* Display it */
        write(STDOUT_FILENO, buffer, size);
    }

    close(fd);
    return EXIT_SUCCESS;
}
