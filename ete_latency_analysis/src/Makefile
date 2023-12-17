CC = gcc
#INSERT PATH TO MOSQUITTO HERE!
CFLAGS = -I"{PATH_TO_MOSQUITTO}/include"
LDFLAGS = -L. -lmosquitto

# Target: all
# Builds both client and server
all: client server 

client:  client.c common.h 
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

server: server.c common.h 
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

clean:
	rm -f client server
