/*
 *  TCP connection sample program
 *   explaining the way to use planetfs
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    int fd, client_fd, size;
    char client_path[1024] = {0}, buffer[65535] = {0};

    /* Establish a server waiting on port 10000 */
    fd = open("./net/tcp/*!10000", O_CREAT | O_RDWR, S_IRWXU);
    if (fd < 0)
        die("open");

    while (1) {
        /* Accept a client's connection */
        size = read(fd, client_path, sizeof (client_path));
        if (size < 0)
            die("read");
        printf("accepted client path: %s\n", client_path);

        /* Open a connection to the client */
        client_fd = open(client_path, O_RDWR);
        if (client_fd < 0)
            die("open");

        /* Receive the response of the remote host */
        do {
            size = read(client_fd, buffer, sizeof (buffer));
            if (size < 0) {
                perror("read");
                break;
            }
            /* Display the response of the server */
            write(STDOUT_FILENO, buffer, size);
        } while (size != 0);
        close(client_fd);
    }
    close(fd);
    return EXIT_SUCCESS;
}
