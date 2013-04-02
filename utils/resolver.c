
#include <iostream>
#include <vector>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

std::vector<std::string> resolved_names_;

int resolver(std::string const& host)
{
    struct addrinfo hints = {}, *result;
    hints.ai_family = AF_UNSPEC;        // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;    // Stream socke

    int s = getaddrinfo(host.c_str(), NULL, &hints, &result);
    if (s != 0) {
        std::cout << "getaddrinfo: " <<  gai_strerror(s) << std::endl;
        return -1;
    }
    for (struct addrinfo *ai = result; ai; ai = ai->ai_next) {
        std::vector<char> buf(1024, 0);
        if (ai->ai_family == AF_INET)
            inet_ntop(ai->ai_family, &((struct sockaddr_in *)ai->ai_addr)->sin_addr, buf.data(), buf.size());
        else if (ai->ai_family == AF_INET6)
            inet_ntop(ai->ai_family, &((struct sockaddr_in6 *)ai->ai_addr)->sin6_addr, buf.data(), buf.size());
        resolved_names_.emplace_back(buf.data());
    }
    freeaddrinfo(result);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 2)
        return 1;
    resolver(argv[1]);
    std::cout << argv[1] << ":" << std::endl;
    for (auto& str : resolved_names_)
        std::cout << str << std::endl;
    return 0;
}
