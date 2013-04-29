
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

    fd = open("./net/eth/ip/tcp/127.0.0.1:10000", O_CREAT | O_RDWR, S_IRWXU);
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

    close(fd);
    return 0;
}
