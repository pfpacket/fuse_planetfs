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

void die(const char *prefix)
{
    perror(prefix);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    int clone_fd, data_fd, size;
    char data_file[256], buffer[1024];
    const char *ctl_request  = "connect 74.125.235.240!80",
               *http_request = "GET /index.html HTTP/1.1\r\n"
                               "Host: www.google.co.jp\r\n"
                               "Connection: close\r\n\r\n";
    clone_fd = open("/net/tcp/clone", O_RDWR);
    if (clone_fd < 0)
        die("clone: open");
    if (read(clone_fd, buffer, sizeof (buffer)) < 0)
        die("clone: read");
    if (write(clone_fd, ctl_request, strlen(ctl_request)) < 0)
        die("clone: write");

    snprintf(data_file, sizeof (data_file), "/net/tcp/%s/data", buffer);
    data_fd = open(data_file, O_RDWR);
    if (data_fd < 0)
        die("data: open");

    /* Send a HTTP request */
    if (write(data_fd, http_request, strlen(http_request)) < 0)
        die("data: write");

    /* Receive the response of the remote host */
    do {
        size = read(data_fd, buffer, sizeof (buffer));
        if (size < 0)
            die("data: read");
        /* Display the response of the server */
        write(STDOUT_FILENO, buffer, size);
    } while (size != 0);

    close(data_fd);
    close(clone_fd);
    return EXIT_SUCCESS;
}
