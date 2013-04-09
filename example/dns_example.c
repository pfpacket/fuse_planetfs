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
    char buffer[1024];
    static char const *request = "resolve_inet www.google.co.jp";

    fd = open("./net/dns", O_RDWR, S_IRWXU);
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
        printf("%s\n", buffer);
    }

    close(fd);
    return EXIT_SUCCESS;
}
