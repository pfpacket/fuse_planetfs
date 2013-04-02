/*
 *  DNS name resolving sample program
 *   explaining the way to use planetfs
 */

#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
    int fd, size;
    char buffer[1024];
    std::string request = "resolve ";
    if (argc < 2)
        return EXIT_FAILURE;

    fd = open("./net/dns", O_RDWR, S_IRWXU);
    if (fd < 0) {
        std::cout << "open: " << std::strerror(errno) << std::endl;
        return EXIT_FAILURE;
    }

    /* Request DNS service */
    request += argv[1];
    size = write(fd, request.c_str(), request.length() + 1);
    if (size < 0) {
        std::cout << "write: " << std::strerror(errno) << std::endl;
        return EXIT_FAILURE;
    }

    /* Read /net/dns to get the result */
    for (;;) {
        size = read(fd, buffer, sizeof (buffer));
        if (size <= 0)  // read error or EOF
            break;
        /* Display it */
        std::cout << buffer << std::endl;
    }

    close(fd);
    return EXIT_SUCCESS;
}
