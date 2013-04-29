
#include <iostream>
#include <string>
#include <algorithm>

void recursive_create(std::string const& path)
{
    for (std::size_t current_pos = 1; current_pos - 1 != std::string::npos; ) {
        auto pos = path.find_first_of('/', current_pos);
        std::cout << path.substr(current_pos, pos - current_pos) << std::endl;
        current_pos = pos + 1;
    }
}

void get_inode_of(std::string const& path)
{
    if (path.empty() || path[0] != '/')
        throw std::string{"empty string passed"};
    auto pos = path.find_first_of('/', 1);
    if (pos == std::string::npos) {
        std::cout << path.substr(1) << std::endl;
        return;
    }
    std::cout << path << " --> " << path.substr(1, pos - 1) << std::endl;
    get_inode_of("/" + path.substr(pos + 1));
}

bool create_directory(std::string const& path)
{
    auto p = path;
    if (p.empty())
        return false;
    if (p[0] != '/')
        p = "/net/dns" + ("/" + path);
    std::cout << "create_directory: " << p << std::endl;
    return true;
}

int main(int argc, char **argv)
{
    std::string path = "/net/eth/ip/tcp";
    std::cout << path << std::endl;
    std::cout << "recursive_create:" << std::endl;
    recursive_create(path);
    std::cout << "get_inode_of:" << std::endl;
    get_inode_of(path);
    create_directory(path);
    create_directory("/net/eth");
    create_directory("net/eth");
    return 0;
}
