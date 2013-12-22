
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

enum constants {
    buffer_size = 1024
};

inline int max(int a, int b)
{
    return a > b ? a : b;
}

void die(int exit_code, const char *msg)
{
    if (msg)
        fprintf(stderr, "FATAL ERROR: %s\n", msg);
    exit(exit_code);
}

inline void display_usage(FILE *fout, const char *exec)
{
    fprintf(fout, "Usage: %s [OPTIONS] NET_PATH\n", exec);
}

int sock_connect_to(const char *host, const char *port)
{
    int s, sock = -1;
    struct addrinfo hints, *ai, *res;
    memset(&hints, 0, sizeof (hints));
    hints.ai_family     = AF_UNSPEC;
    hints.ai_socktype   = SOCK_STREAM;
    hints.ai_flags      = AI_PASSIVE;

    s = getaddrinfo(host, port, &hints, &res);
    if (s != 0)
        return -1;

    for (ai = res; ai; ai = ai->ai_next) {
        sock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (sock < 0)
            continue;
        if (connect(sock, ai->ai_addr, ai->ai_addrlen) < 0)
            continue;
        break;
    }
    return sock;
}

void read_and_write(int in, int out)
{
    char buf[buffer_size];
    int backup_flag = fcntl(in, F_GETFL);
    fcntl(in, F_SETFL, backup_flag | O_NONBLOCK);
    for (;;) {
        int ret = read(in, buf, sizeof buf);
        if (ret <= 0)
            break;
        write(out, buf, ret);
    }
    fcntl(in, F_SETFL, backup_flag);
}

int read_write_until_eof(int fd)
{
    int ret = 0;
    fd_set rfds, backup_fds;
    FD_ZERO(&backup_fds);
    FD_SET(fd, &backup_fds);
    FD_SET(STDIN_FILENO, &backup_fds);
    for (;;) {
        rfds = backup_fds;
        int res = select(max(STDIN_FILENO, fd) + 1, &rfds, NULL, NULL, NULL);
        if (res == -1) {
            if (errno == EINTR)
                continue;
            die(EXIT_FAILURE, strerror(errno));
        }
        if (FD_ISSET(fd, &rfds))
            read_and_write(fd, STDOUT_FILENO);
        if (FD_ISSET(STDIN_FILENO, &rfds))
            read_and_write(STDIN_FILENO, fd);
    }
    return ret;
}

int main(int argc, char **argv)
{
    int fd;
    if (argc <= 1) {
        display_usage(stderr, argv[0]);
        die(EXIT_FAILURE, "Too few arguments");
    }
    fd = open(argv[1], O_RDWR);
    //fd = sock_connect_to(argv[1], "80");
    if (fd == -1)
        die(EXIT_FAILURE, strerror(errno));
    printf("Successfully connected to %s:80\n", argv[1]);
    return read_write_until_eof(fd);
}
