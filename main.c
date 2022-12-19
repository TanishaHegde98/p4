#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mfs.h"

int main(int argc, char *argv[]) {
    int port=atoi(argv[1]);
    int rc = MFS_Init("localhost", port);
    rc = MFS_Creat(0,1,"test.txt");
    int num = MFS_Lookup(0,"test.txt");
    char buff[4096] = "Test Buffer";
    char rcv[4096] = {0};
    MFS_Write(num,buff,0,4096);
    MFS_Read(num,rcv,0,4096);
    printf("read back %s",rcv);
    int o = strcmp(buff,rcv);
    printf("out put %d\n",o);
    MFS_Stat_t m;
    MFS_Stat(num,&m);
    printf("size of file %d",m.size);
    rc = MFS_Shutdown();
    return 0;
}
