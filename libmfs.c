#include <stdio.h>
#include "mfs.h"
#include "udp.h"
#include <time.h>
#include <stdlib.h>

#define MIN_PORT 20000
#define MAX_PORT 40000

#define BUFFER_SIZE (1000)
#define MFS_LOOKUP "1"
#define MFS_STAT "2"
#define MFS_WRITE "3"
#define MFS_READ "4"
#define MFS_CREAT "5"
#define MFS_UNLINK "6"
#define MFS_SHUTDOWN "7"

    typedef struct {
	unsigned char command[BUFFER_SIZE];
    char data[4096];
    } block_t;
int sd;
struct sockaddr_in addrSnd, addrRcv;
int MFS_Init(char *hostname, int port)
{
    srand(time(0));
    int port_num = (rand() % (MAX_PORT - MIN_PORT) + MIN_PORT);
    printf("Client running UDP on port : %d", port_num);
    sd = UDP_Open(port_num);
    int rc = UDP_FillSockAddr(&addrSnd, hostname, port);

    if (rc < 0)
    {
        printf("client:: could not connect to server\n");
        exit(1);
    }
    printf("Connected to server");
    // char message[BUFFER_SIZE];
    // sprintf(message, "hello world");

    // printf("client:: send message [%s]\n", message);
    // rc = UDP_Write(sd, &addrSnd, message, BUFFER_SIZE);
    // if (rc < 0) {
    // printf("client:: failed to send\n");
    // exit(1);
    //}

    // printf("client:: wait for reply...\n");
    // rc = UDP_Read(sd, &addrRcv, message, BUFFER_SIZE);
    // printf("client:: got reply [size:%d contents:(%s)\n", rc, message);
    // return 0;
    // do some net setup
    return 0;
}

int MFS_Lookup(int pinum, char *name)
{
    // network communication to do the lookup to server
    // struct sockaddr_in addrSnd, addrRcv;
    char command[BUFFER_SIZE] = {0};
    char received[BUFFER_SIZE];
    char str_int[10];

    strcat(command, MFS_LOOKUP);
    strcat(command, ":");
    sprintf(str_int, "%d", pinum);
    strcat(command, str_int);
    strcat(command, ":");
    strcat(command, name);

    // printf("\ncommand: %s",command);
    int rc = UDP_Write(sd, &addrSnd, command, BUFFER_SIZE);
    if (rc < 0)
    {
        printf("client:: failed to send\n");
    }
    rc = UDP_Read(sd, &addrRcv, received, BUFFER_SIZE);
    rc = atoi(received);
    return rc;
}

int MFS_Stat(int inum, MFS_Stat_t *m)
{
    char command[BUFFER_SIZE] = {0};
    char str_int[10];
    strcat(command, MFS_STAT);
    strcat(command, ":");
    sprintf(str_int, "%d", inum);
    strcat(command, str_int);
    UDP_Write(sd, &addrSnd, command, BUFFER_SIZE);
    char buf[BUFFER_SIZE];
    UDP_Read(sd, &addrRcv, buf, BUFFER_SIZE);
    // Tokenize

    char *pch;
    char *OP[2];
    int i = 0;
    pch = strtok(buf, ":");
    while (pch != NULL)
    {
        OP[i++] = strdup(pch);
        pch = strtok(NULL, ":");
    }
    strtok(buf, ":");
    m->type = atoi(OP[0]);
    m->size = atoi(OP[1]);
   // printf("Libmfs: \n");
    //printf("type:%d", m->type);
   // printf("size:%d\n", m->size);
    return 0;
}

int MFS_Write(int inum, char *buffer, int offset, int nbytes)
{
    char command[8000] = {0};
    char received[BUFFER_SIZE];
    char str_int[10];

    //printf("libmfs rcv: %s\n",buffer);
    strcat(command, MFS_WRITE);
    strcat(command, ":");
    sprintf(str_int, "%d", inum);
    strcat(command, str_int);
    strcat(command, ":");
    strcat(command, buffer);
    strcat(command, ":");
    sprintf(str_int, "%d", offset);
    strcat(command, str_int);
    strcat(command, ":");
    sprintf(str_int, "%d", nbytes);
    strcat(command, str_int);
    int rc = UDP_Write(sd, &addrSnd, (char *)command,6000 );
    if (rc < 0)
    {
        printf("client:: failed to send\n");
    }
    rc = UDP_Read(sd, &addrSnd, received, BUFFER_SIZE);
    rc = atoi(received);
    return rc;
}

int MFS_Read(int inum, char *buffer, int offset, int nbytes)
{
    char command[BUFFER_SIZE] = {0};
    char received[8000]={0};
    char str_int[10];

    strcat(command, MFS_READ);
    strcat(command, ":");
    sprintf(str_int, "%d", inum);
    strcat(command, str_int);
    strcat(command, ":");
    sprintf(str_int, "%d", offset);
    strcat(command, str_int);
    strcat(command, ":");
    sprintf(str_int, "%d", nbytes);
    strcat(command, str_int);

    //printf("command: %s", command);

    int rc = UDP_Write(sd, &addrSnd, command, BUFFER_SIZE);
    if (rc < 0)
    {
        printf("client:: failed to send\n");
    }
    rc = UDP_Read(sd, &addrSnd, received, 8000);
    memcpy(buffer, received, nbytes);
    return 0;
}

int MFS_Creat(int pinum, int type, char *name)
{
    char command[BUFFER_SIZE] = {0};
    char received[BUFFER_SIZE];
    char str_int[10];

    strcat(command, MFS_CREAT);
    strcat(command, ":");
    sprintf(str_int, "%d", pinum);
    strcat(command, str_int);
    strcat(command, ":");
    sprintf(str_int, "%d", type);
    strcat(command, str_int);
    strcat(command, ":");
    strcat(command, name);

    // printf("command: %s",command);
    int rc = UDP_Write(sd, &addrSnd, command, BUFFER_SIZE);
    if (rc < 0)
    {
        printf("client:: failed to send\n");
    }
    rc = UDP_Read(sd, &addrSnd, received, BUFFER_SIZE);
    rc = atoi(received);
    return rc;
}

int MFS_Unlink(int pinum, char *name)
{
    char command[BUFFER_SIZE] = {0};
    char received[BUFFER_SIZE] = {0};
    char str_int[10];
    strcat(command, MFS_UNLINK);
    strcat(command, ":");
    sprintf(str_int, "%d", pinum);
    strcat(command, str_int);
    strcat(command, ":");
    strcat(command, name);

    //printf("command: %s", command);
    int rc = UDP_Write(sd, &addrSnd, command, BUFFER_SIZE);
    if (rc < 0)
    {
        printf("client:: failed to send\n");
    }
    rc = UDP_Read(sd, &addrRcv, received, BUFFER_SIZE);
    rc = atoi(received);
    return rc;
}

int MFS_Shutdown()
{
    // printf("MFS Shutdown\n");
    char command[BUFFER_SIZE] = {0};
    // char received[BUFFER_SIZE];
    strcat(command, MFS_SHUTDOWN);
    // printf("\ncommand: %s",command);
    int rc = UDP_Write(sd, &addrSnd, command, BUFFER_SIZE);
    if (rc < 0)
    {
        printf("client:: failed to send\n");
    }
    // Server shut down so cannot get response
    return 0;
}
