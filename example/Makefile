
CFLAGS     = -Wall -O2
TARGETS    = dns_example \
			 http_client_example \
			 tcp_server_example \
			 packet_recv_example

all: $(TARGETS)

dns_example: dns_example.o
	$(CC) $(LDFLAGS) -o $@ dns_example.o $(LIBS)
http_client_example : http_client_example.o
	$(CC) $(LDFLAGS) -o $@ http_client_example.o $(LIBS)
tcp_server_example: tcp_server_example.o
	$(CC) $(LDFLAGS) -o $@ tcp_server_example.o $(LIBS)
packet_recv_example: packet_recv_example.o
	$(CC) $(LDFLAGS) -o $@ packet_recv_example.o $(LIBS)

clean:
	rm -f $(TARGETS) *.o
