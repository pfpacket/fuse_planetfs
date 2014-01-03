#!/usr/bin/env python3
#
#   Simple HTTP client written in Python3
#       Showing how to use planetfs
#
import sys

class raw_io:
    def __init__(self, *args, **kwargs):
        self.open_(*args, **kwargs)
    @staticmethod
    def open(*args, **kwargs):
        return raw_io(*args, **kwargs)
    def open_(self, filename, mode, *args, **kwargs):
        self.file = open(filename, mode + "b", buffering=0, *args, **kwargs)
    def read(self, *args, **kwargs):
        return self.file.read(*args, **kwargs).decode()
    def write(self, buf, *args, **kwargs):
        return self.file.write(buf.encode(), *args, **kwargs)
    def close(self, *args, **kwargs):
        return self.file.close(*args, **kwargs)
    def get(self):
        return self.file

def resolve_name(hostname):
    dns = raw_io.open("/net/dns", "r+")
    dns.write("resolve_inet " + hostname)
    return dns.read().splitlines()

def simple_http_client(hostname):
    resolved = resolve_name(hostname)

    tcp_clone = raw_io.open("/net/tcp/clone", "r")
    clone_num = tcp_clone.read()
    tcp_ctl_path = "/net/tcp/" + clone_num + "/ctl"

    tcp_ctl = raw_io.open(tcp_ctl_path, "w")
    tcp_ctl.write("connect " + resolved[0] + "!80")

    tcp_data = raw_io.open("/net/tcp/" + clone_num + "/data", "r+")
    tcp_data.write(
        "GET /index.html HTTP/1.0\r\nHost: {0}\r\n\r\n"
    .format(hostname))
    http_response = tcp_data.get().read()
    tcp_ctl.write("hangup")
    return http_response

if __name__ == "__main__":
    sys.stdout.buffer.write(simple_http_client(
        "www.google.com" if len(sys.argv) < 2 else sys.argv[1]
    ))
