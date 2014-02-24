
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/limits.h>

#define DEFINE_SAFE_SYSCALL(name, type1, arg1, type2, arg2, type3, arg3) \
    int safe_##name(type1 arg1, type2 arg2, type3 arg3) { \
        int ret = name(arg1, arg2, arg3); \
        if (ret < 0) { \
            perror(#name); \
            exit(EXIT_FAILURE); \
        } \
        return ret; \
    }
DEFINE_SAFE_SYSCALL(open, const char *, path, int, flags, int, mode)
DEFINE_SAFE_SYSCALL(read, int, fd, char *, buffer, int, size)
DEFINE_SAFE_SYSCALL(write, int, fd, const char *, buffer, int, size)

void establish_server(const char port[], char *listen_path, int path_size)
{
    char session[512], announce_req[512];
    int size, clone = safe_open("/net/tcp/clone", O_RDWR, 0);
    size = safe_read(clone, session, sizeof session);
    session[size - 1] = '\0';
    snprintf(announce_req, sizeof announce_req, "announce *!%s", port);
    snprintf(listen_path, path_size, "/net/tcp/%s/listen", session);
    safe_write(clone, announce_req, strlen(announce_req));
}

int accept_client(const char listen_path[])
{
    char new_session[PATH_MAX], client_data[PATH_MAX];
    int listen = safe_open(listen_path, O_RDONLY, 0);
    int size = safe_read(listen, new_session, sizeof new_session);
    new_session[size - 1] = '\0';
    snprintf(client_data, sizeof client_data, "/net/tcp/%s/data", new_session);
    return safe_open(client_data, O_RDWR, 0);
}

void echo_server(int fd)
{
    int size;
    char buffer[512];
    for (; (size = safe_read(fd, buffer, sizeof buffer)); )
        safe_write(fd, buffer, size);
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
    int client, ret;
    char listen_path[PATH_MAX];
    if (argc < 2) {
        printf("Usage: %s LISTEN_PORT\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    signal(SIGCHLD, SIG_IGN);
    establish_server(argv[1], listen_path, sizeof listen_path);
    for (; ; close(client)) {
        client = accept_client(listen_path);
        ret = fork();
        if (ret == 0)
            echo_server(client);
        else if (ret < 0)
            perror("fork");
    }
    return EXIT_SUCCESS;
}
