#include <stdio.h>
#include "mfs.h"
#include "udp.h"

#define BUFFER_SIZE (1000)
#define MFS_LOOKUP "1"
#define MFS_STAT "2"
#define MFS_WRITE "3"
#define MFS_READ "4"
#define MFS_CREAT "5"

int sd;
struct sockaddr_in addrSnd, addrRcv;
int MFS_Init(char *hostname, int port) {
    printf("MFS Init2n a  %s %d\n", hostname, port);
    
    sd = UDP_Open(20000);
    int rc = UDP_FillSockAddr(&addrSnd, hostname, port);

    if( rc < 0){
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

int MFS_Lookup(int pinum, char *name) {
    // network communication to do the lookup to server
    //struct sockaddr_in addrSnd, addrRcv;
    char command[BUFFER_SIZE] = {0};
    char received[BUFFER_SIZE];
    char str_int[10];
    
    strcat(command,MFS_LOOKUP);
    strcat(command, ":");
    sprintf(str_int,"%d",pinum);
    strcat(command,str_int);
    strcat(command, ":");
    strcat(command,name);
    
    
    printf("command: %s",command);
    int rc = UDP_Write(sd,&addrSnd,command, BUFFER_SIZE);
    if (rc < 0) {
	    printf("client:: failed to send\n");
    }
    rc = UDP_Read(sd,&addrSnd,received,BUFFER_SIZE);
    rc = atoi(received);
    return rc;
}

int MFS_Stat(int inum, MFS_Stat_t *m) {
    return 0;
}

int MFS_Write(int inum, char *buffer, int offset, int nbytes) {
    return 0;
}

int MFS_Read(int inum, char *buffer, int offset, int nbytes) {
    return 0;
}

int MFS_Creat(int pinum, int type, char *name) {
    char command[BUFFER_SIZE] = {0};
    char received[BUFFER_SIZE];
    char str_int[10];
    
    strcat(command,MFS_CREAT);
    strcat(command, ":");
    sprintf(str_int,"%d",pinum);
    strcat(command,str_int);
    strcat(command, ":");
    sprintf(str_int,"%d",type);
    strcat(command,str_int);
    strcat(command, ":");
    strcat(command,name);
    
    
    printf("command: %s",command);
    int rc = UDP_Write(sd,&addrSnd,command, BUFFER_SIZE);
    if (rc < 0) {
	    printf("client:: failed to send\n");
    }
    rc = UDP_Read(sd,&addrSnd,received,BUFFER_SIZE);
    rc = atoi(received);
    return rc;

    return 0;
}

int MFS_Unlink(int pinum, char *name) {
    return 0;
}

int MFS_Shutdown() {
    printf("MFS Shutdown\n");
    return 0;
}
