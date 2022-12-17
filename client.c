#include <stdio.h>
#include "udp.h"
#include <time.h>
#include <stdlib.h>

int MIN_PORT = 20000;
int MAX_PORT = 40000;

#define BUFFER_SIZE (1000)

// client code
int main(int argc, char *argv[]) {
    struct sockaddr_in addrSnd, addrRcv;
    int port=atoi(argv[1]);
    srand(time(0));
    int port_num = (rand() % (MAX_PORT - MIN_PORT) + MIN_PORT);
    int sd = UDP_Open(port_num);
    int rc = UDP_FillSockAddr(&addrSnd, "localhost", port);

    char message[BUFFER_SIZE];
    sprintf(message, "hello world");

    printf("client:: send message [%s]\n", message);
    rc = UDP_Write(sd, &addrSnd, message, BUFFER_SIZE);
    if (rc < 0) {
	printf("client:: failed to send\n");
	exit(1);
    }

    printf("client:: wait for reply...\n");
    rc = UDP_Read(sd, &addrRcv, message, BUFFER_SIZE);
    printf("client:: got reply [size:%d contents:(%s)\n", rc, message);
    return 0;
}
