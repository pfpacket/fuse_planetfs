
CFLAGS     = -Wall -O2
TARGETS    = dns_example \
			 http_client_example \
			 new_http_client \
			 tcp_server_example \
			 packet_recv_example \
			 netcat \
			 pkt_rcv_select
LIBS       =
LDFLAGS    =

all: $(TARGETS)

dns_example: dns_example.o
	$(CC) $(LDFLAGS) -o $@ dns_example.o $(LIBS)
http_client_example : http_client_example.o
	$(CC) $(LDFLAGS) -o $@ http_client_example.o $(LIBS)
new_http_client: new_http_client.o
	$(CC) $(LDFLAGS) -o $@ new_http_client.o $(LIBS)
tcp_server_example: tcp_server_example.o
	$(CC) $(LDFLAGS) -o $@ tcp_server_example.o $(LIBS)
packet_recv_example: packet_recv_example.o
	$(CC) $(LDFLAGS) -o $@ packet_recv_example.o $(LIBS)
netcat: netcat.o
	$(CC) $(LDFLAGS) -o $@ netcat.o $(LIBS)
pkt_rcv_select: pkt_rcv_select.o
	$(CC) $(LDFLAGS) -o $@ pkt_rcv_select.o $(LIBS)

clean:
	rm -f $(TARGETS) *.o
