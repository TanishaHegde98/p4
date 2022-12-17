#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mfs.h"

int main(int argc, char *argv[]) {
    int port=atoi(argv[1]);
    int rc = MFS_Init("localhost", port);
    printf("init %d\n", rc);

    //printf("lookup %d\n", rc);

    //rc = MFS_Creat(0,0,"_newdir");
    //rc = MFS_Creat(1,0,"new_in_newdir");
    rc = MFS_Creat(0,0,"testDir");
    printf("\nRC:%d",rc);
    rc = MFS_Creat(1,0,"/testDir/test.txt");
    printf("\nRC:%d",rc);
    //rc = MFS_Lookup(1,".");
    //rc = MFS_Lookup(1,"new_in_newdir");
    rc = MFS_Lookup(0,"testDir");
    printf("\nRC:%d",rc);
    rc = MFS_Shutdown();
    printf("\nRC:%d",rc);

    printf("shut %d\n", rc);
    return 0;
}
