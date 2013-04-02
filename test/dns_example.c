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
    if (argc < 2)
        return EXIT_FAILURE;

    fd = open("./net/dns", O_RDWR, S_IRWXU);
    if (fd < 0) {
        perror("open");
        return EXIT_FAILURE;
    }

    /* Request DNS service */
    size = write(fd, argv[1], strlen(argv[1]));
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
