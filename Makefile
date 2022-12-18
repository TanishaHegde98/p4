CC   = gcc
OPTS = -Wall

all: server lib client

server: server.o udp.o
	$(CC) -o server server.o udp.c libmfs.c

client: main.o udp.o
	$(CC) -o main main.o udp.c libmfs.c

lib: libmfs.o
	$(CC) -Wall -Werror -shared -fPIC -g -o libmfs.so libmfs.c udp.o
	export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH

#bash ./checkEnv.sh

clean:
	rm -f server.o main.o udp.o libmfs.so server client lib