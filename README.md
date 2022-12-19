# p4
We have created a Distributed File System where the client communicates with the server which processes all the file related requests.
We have implemented the following operations:
    int MFS_Init(char *hostname, int port):initializes the UDP connection
    int MFS_Lookup(int pinum, char *name): Locates the file
    int MFS_Stat(int inum, MFS_Stat_t *m): Retrieve file information like type and size
    int MFS_Write(int inum, char *buffer, int offset, int nbytes): Write into a file or directory
    int MFS_Read(int inum, char *buffer, int offset, int nbytes): Read from a file or directory
    int MFS_Creat(int pinum, int type, char *name): Creates a new file or directory
    int MFS_Unlink(int pinum, char *name): Deletes the file or directory
    int MFS_Shutdown(): Shutdown the server
