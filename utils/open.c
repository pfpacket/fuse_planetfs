
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
    if (argc < 2) {
        fputs("Too few arguments", stderr);
        return EXIT_FAILURE;
    }
    int fd = open(argv[1], O_RDONLY | O_CREAT, S_IRWXU | S_IRWXG);
    close(fd);
    return EXIT_SUCCESS;
}
