#include <stdio.h>

#include "mfs.h"

int main(int argc, char *argv[]) {
    int rc = MFS_Init("localhost", 10000);
    printf("init %d\n", rc);

    //printf("lookup %d\n", rc);

    //rc = MFS_Creat(0,0,"_newdir");
    //rc = MFS_Creat(1,0,"new_in_newdir");
    rc = MFS_Creat(1,0,"testDir");
    //rc = MFS_Lookup(1,".");
    //rc = MFS_Lookup(1,"new_in_newdir");
    rc = MFS_Lookup(1,"test.txt");
    rc = MFS_Shutdown();
    printf("shut %d\n", rc);
    return 0;
}
