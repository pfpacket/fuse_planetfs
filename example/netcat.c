
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <fcntl.h>

enum { buffer_size = 1024 };
#define max(a, b) ((a) > (b) ? (a) : (b))

void die(int exit_code, const char *msg)
{
    fprintf(stderr, "Fatal error: %s\n", msg);
    exit(exit_code);
}

int write_once(int in, int out)
{
    char buf[buffer_size];
    int ret = read(in, buf, sizeof buf);
    if (ret > 0)
        write(out, buf, ret);
    return ret;
}

void netcat(int fd)
{
    fd_set rfds, backup_fds;
    FD_ZERO(&backup_fds);
    FD_SET(fd, &backup_fds);
    FD_SET(STDIN_FILENO, &backup_fds);
    for (;;) {
        rfds = backup_fds;
        if (select(max(STDIN_FILENO, fd) + 1, &rfds, NULL, NULL, NULL) < 0) {
            if (errno == EINTR)
                continue;
            die(EXIT_FAILURE, strerror(errno));
        }
        if (FD_ISSET(fd, &rfds))
            if (!write_once(fd, STDOUT_FILENO))
                break;
        if (FD_ISSET(STDIN_FILENO, &rfds))
            if (!write_once(STDIN_FILENO, fd))
                break;
    }
}

int main(int argc, char **argv)
{
    int fd;
    if (argc < 2) {
        fprintf(stderr, "Usage: %s [OPTIONS] NET_PATH\n", argv[0]);
        die(EXIT_FAILURE, "Too few arguments");
    }
    fd = open(argv[1], O_RDWR);
    if (fd == -1)
        die(EXIT_FAILURE, strerror(errno));
    netcat(fd);
    close(fd);
    return 0;
}
