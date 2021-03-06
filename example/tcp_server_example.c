/*
 *  TCP connection sample program
 *   explaining the way to use planetfs
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void die(char const *prefix)
{
    perror(prefix);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    char buffer[65535];
    int client_fd, size;

    /* Establish a server waiting on port 10000 */
    if (mknod("/net/tcp/*!10000", S_IFREG | S_IRWXU, 0) < 0 && errno != EEXIST)
        die("mknod");

    while (1) {
        /* Accept a client's connection */
        client_fd = open("/net/tcp/*!10000", O_RDWR);
        if (client_fd < 0)
            die("read");

        /* Receive the response of the remote host */
        do {
            size = read(client_fd, buffer, sizeof (buffer));
            if (size < 0)
                die("read");
            /* Display the response of the server */
            write(STDOUT_FILENO, buffer, size);
        } while (size != 0);
        close(client_fd);
    }
    remove("/net/tcp/*!10000");
    return EXIT_SUCCESS;
}
