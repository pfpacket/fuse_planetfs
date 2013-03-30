
//
// TCP connection sample program
//  explaining the way to use planetfs
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFFER_SIZE 65535

int main(int argc, char **argv)
{
    int fd, size;
    char buffer[BUFFER_SIZE] = {0};
    static char const *request = 
            "GET /index.html HTTP/1.1\r\n"
            "Host: www.google.co.jp\r\n"
            "Connection: close\r\n\r\n";

    printf("[*] Connecting to www.google.co.jp ...\n");
    // open system call will connect to the remote host specified in the path
    // open system call with O_CREAT will create the end point to remote host
    fd = open("./net/eth/ip/tcp/74.125.235.247:80", O_CREAT | O_RDWR, S_IRWXU);
    if (fd < 0) {
        perror("open");
        return EXIT_FAILURE;
    }
    printf("[*] Connection established fd=%d\n", fd);

    // Send a HTTP request
    printf("[*] Sending HTTP request ...\n");
    size = write(fd, request, strlen(request));
    if (size < 0) {
        perror("write");
        return EXIT_FAILURE;
    }
    printf("[*] Sent out %d bytes\n", size);

    // Receive the response of the remote host
    printf("[*] Receiving a response of the server ...\n");
    size = read(fd, buffer, sizeof (buffer));
    if (size < 0) {
        perror("read");
        return EXIT_FAILURE;
    }
    printf("[*] Received %d bytes\n", size);

    // Display the response of the server
    printf("[*] Now displaying the response:\n");
    write(STDOUT_FILENO, buffer, size);

    close(fd);
    return EXIT_SUCCESS;
}
