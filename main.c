#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mfs.h"

int main(int argc, char *argv[]) {
    int port=atoi(argv[1]);
    int rc = MFS_Init("localhost", port);
    rc = MFS_Creat(0,0,"testDir3");
    int inum = MFS_Lookup(0,"testDir3");
    MFS_Creat(inum,1,"t2.c");
    // printf("Received inum %d\n", inum);
    // MFS_Stat_t m;
    // MFS_Stat(inum,&m);
    rc = MFS_Unlink(0,"testDir3"); 
    printf("rc unlink %d\n",rc);
    printf("rc %d",rc);
    rc=MFS_Unlink(inum,"test2.c");
    printf("rc test2.c %d\n",rc);
    rc =MFS_Unlink(0,"testDir3"); 
    printf("rc testDir3 %d\n",rc);
    //printf("m.type %d m.size %d", m.type, m.size);
    rc = MFS_Shutdown();
    return 0;
}
