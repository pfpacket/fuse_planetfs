#!/usr/bin/env python3
#
#   Simple HTTP client written in Python3
#       Showing how to use planetfs
#
import sys

def simple_http_client(hostname, path):
    dns = open("/net/dns", "rb+", buffering=0)
    dns.write(("resolve_inet " + hostname).encode())
    resolved = dns.read().decode().splitlines()

    tcp_clone = open("/net/tcp/clone", "rb", buffering=0)
    clone_num = tcp_clone.read().decode()

    tcp_ctl = open("/net/tcp/" + clone_num + "/ctl", "wb", buffering=0)
    tcp_ctl.write("connect {0}!80".format(resolved[0]).encode())

    tcp_data = open("/net/tcp/" + clone_num + "/data", "rb+", buffering=0)
    tcp_data.write(
        "GET {0} HTTP/1.1\r\nHost: {1}\r\nConnection: close\r\n\r\n"
    .format(path, hostname).encode())
    http_response = tcp_data.read()
    tcp_ctl.write("hangup".encode())
    return http_response

if __name__ == "__main__":
    if (len(sys.argv) < 2):
        sys.exit("Usage: ./{0} HOSTNAME [PATH]".format(sys.argv[0]))
    sys.stdout.buffer.write(simple_http_client(
        hostname=sys.argv[1], path=(sys.argv[2] if len(sys.argv) > 2 else "/")
    ))
