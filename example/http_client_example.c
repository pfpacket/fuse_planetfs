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

int main(int argc, char **argv)
{
    int fd, size;
    char buffer[65535] = {0};
    static char const *request = 
                "GET /index.html HTTP/1.1\r\n"
                "Host: www.google.co.jp\r\n"
                "Connection: close\r\n\r\n";

    /* open system call will connect to the remote host specified in the path */
    /* open system call with O_CREAT will create the end point to remote host */
    fd = open("/net/tcp/74.125.235.240!80", O_CREAT | O_RDWR, S_IRWXU);
    if (fd < 0) {
        perror("open");
        return EXIT_FAILURE;
    }

    /* Send a HTTP request */
    size = write(fd, request, strlen(request));
    if (size < 0) {
        perror("write");
        return EXIT_FAILURE;
    }

    /* Receive the response of the remote host */
    do {
        size = read(fd, buffer, sizeof (buffer));
        if (size < 0) {
            perror("read");
            break;
        }
        /* Display the response of the server */
        write(STDOUT_FILENO, buffer, size);
    } while (size != 0);

    close(fd);
    return EXIT_SUCCESS;
}
