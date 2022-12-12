#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "udp.h"
#include "ufs.h"

#define BUFFER_SIZE (1000)
#define MAXLIST 10
#define INUM_PER_BLOCK (UFS_BLOCK_SIZE / sizeof(inode_t))
#define MAX_FNAME 28
#define DIRECTORY_ENTRIES 128

// helper structs
typedef struct
{
    inode_t inodes[UFS_BLOCK_SIZE / sizeof(inode_t)];
} inode_block;

typedef struct
{
    dir_ent_t entries[128];
} dir_block_t;

typedef struct
{
    unsigned int bits[UFS_BLOCK_SIZE / sizeof(unsigned int)];
} bitmap_t;

// server code
super_t *s;
int fd;

// block functions

int read_block(int blk, void *block)
{

    off_t offset = blk * UFS_BLOCK_SIZE;
    lseek(fd, offset, SEEK_SET);
    int size = read(fd, block, UFS_BLOCK_SIZE);
    if (size != UFS_BLOCK_SIZE)
    {
        printf("Failed to read entire block");
    }
    return 0;
}

int write_block(int blk, void *block){
    off_t offset = blk * UFS_BLOCK_SIZE;
    lseek(fd,offset,SEEK_SET);
    int size = write(fd, block, UFS_BLOCK_SIZE);
    fsync(fd);
    if (size != UFS_BLOCK_SIZE)
    {
        printf("Failed to read entire block");
    }
    return 0;
}

int get_datablock(int pinum, int *cur_entry, int itype)
{
    int blk = (pinum / INUM_PER_BLOCK) + s->inode_region_addr;
    int dataBlk;
    int pinum_in_blk = pinum % INUM_PER_BLOCK;

    // TODO: check bitmap of inode is valid
    // Read blk containing inode
    inode_block iBlock;
    read_block(blk, (void *)&iBlock);
    //printf("pinum_in_blk %d\n", pinum_in_blk);

    for (int i = *cur_entry; i < DIRECT_PTRS; i++)
    {
        //printf("iBlock.inodes[pinum_in_blk].direct[i] %d\n", iBlock.inodes[pinum_in_blk].direct[i]);

        // if inode is not of desired type -1 if can be of either type
        if (itype != -1 && iBlock.inodes[pinum_in_blk].type != itype)
            return -1;
        if (iBlock.inodes[pinum_in_blk].direct[i] == -1)
            return -1;
        //printf("Found datablock");
        dataBlk = iBlock.inodes[pinum_in_blk].direct[i];
        *cur_entry = i+1;
        return dataBlk;
    }
    return -1;
}

int get_free_bitmap(int blk){
    bitmap_t bitmap;
    read_block(blk, (void *)&bitmap);

    for (int byte = 0; byte < 1024; byte++)
    {

        // check to see if byte has available slot
        if (bitmap.bits[byte] != 0xFFFFFFFF)
        {

            // loop to check each bit
            for (int bit = 0; bit < 32; bit++)
            {
                int mask = 0x80000000 >> bit;
                if (mask & ~bitmap.bits[byte])
                {

                    // Available block is found: set bit in bitmap, write result back
                    // to data bitmap block, and return block number.
                    bitmap.bits[byte] |= mask;
                    write_block(blk, (void *)&bitmap);
                    //printf("\nbyte: %d, bits: %d\n",byte,bit);

                    return (byte * 1024) + bit;
                }
            }
        }
    }
    return -1;
}

// int get_free_block()
// {

//     // check data bit map
//     // change the bit

//     }
//     return -1; // disk full
// }

// mfs Code
int mfs_lookup(char **argList, int *argSize)
{

    if (*argSize < 3)
        return -1;

    int pinum = atoi(argList[1]);
    int dataBlk = 0, cur_dir_entry = 0;
    char *name = argList[2];
    dir_block_t dEntries;
    //printf("pinum in mfs_lookup: %d\n",pinum);
    while (1)
    {
        dataBlk = get_datablock(pinum, &cur_dir_entry, 0);
        //printf("dataBlk: %d\n",dataBlk);
        if (dataBlk == -1)
            return -1;
        read_block(dataBlk, (void *)&dEntries);
        for (int i = 0; i < 128; i++)
        {  // if (dEntries.entries[i].inum != -1){
        //         printf("dEntries.entries[i].name: %s\n",dEntries.entries[i].name);
        //         printf("dEntries.entries[i].inum  %d\n",dEntries.entries[i].inum );
        //     }
            if ((dEntries.entries[i].inum != -1) && (strcmp(dEntries.entries[i].name, name) == 0))
            {
                printf("Found file %s", dEntries.entries[i].name);
                return 0;
            }
        }
        printf("did not find file\n");
    }
    return 0;
}

int mfs_stat(char **argList, int *argSize)
{
    return 0;
}

int mfs_write(char **argList, int *argSize)
{
    return 0;
}

int mfs_read(char **argList, int *argSize)
{
    return 0;
}

int mfs_creat(char **argList, int *argSize)
{
    if (*argSize < 4)
        return -1;

    int pinum = atoi(argList[1]);
    //printf("pinum in mfs_create: %d\n",pinum);
    int type = atoi(argList[2]);
    int parentBlk = 0, cur_dir_entry = 0,i=0;
    char *name = argList[3];
    dir_block_t dEntries;

    while (1)
    {
        parentBlk = get_datablock(pinum, &cur_dir_entry, 0);
        if (parentBlk == -1)
            return -1;
        read_block(parentBlk, (void *)&dEntries);
        for (int i = 0; i < DIRECTORY_ENTRIES; i++)
        {
            if ((dEntries.entries[i].inum != -1) && (strcmp(dEntries.entries[i].name, name) == 0))
            {
                //printf("Creat Found file %s", dEntries.entries[i].name);
                return 0;
            }
        }

        if (strlen(name) > MAX_FNAME)
            return -1;

        //TODO: usage of bitmap length
        printf("parent block: %d\n",parentBlk);
        int new_blk_num = get_free_bitmap(s->data_bitmap_addr);
        new_blk_num += s->data_region_addr;
        printf("new_blk_num: %d",new_blk_num);
        // get inode number
        int newInode = get_free_bitmap(s->inode_bitmap_addr);
        printf("\nNew Inode num: %d",newInode);
        //update current directory
        for (i = 0; i < DIRECTORY_ENTRIES; i++)
        {
            if(dEntries.entries[i].inum!= -1)
                continue;
            dEntries.entries[i].inum = newInode;
            strcpy(dEntries.entries[i].name,name);
            write_block(parentBlk,(void *)&dEntries);
            printf("dEntries.entries[i].name: %s\n",dEntries.entries[i].name);
            break;
        }
        
        if(i==DIRECTORY_ENTRIES)
        {
            printf("Directory full");
            return -1;
        }

        //update inum data
        inode_block itable;
        read_block(s->inode_region_addr,(void *)&itable);
        itable.inodes[newInode].type = type;
        itable.inodes[newInode].direct[0] = new_blk_num;
        for (int j=1; j < DIRECT_PTRS; j++){
            itable.inodes[newInode].direct[j] = -1;
        }
        if(type == UFS_DIRECTORY){
            itable.inodes[newInode].size = sizeof(dir_ent_t);
            write_block(s->inode_region_addr,(void *)&itable);
            dir_block_t newDblk;
            read_block(new_blk_num, (void *)&newDblk);
            strcpy(newDblk.entries[0].name,".");
            newDblk.entries[0].inum = newInode;
            strcpy(newDblk.entries[1].name,"..");
            newDblk.entries[1].inum = pinum;
            for (int j = 2; j < 128; j++){
	            newDblk.entries[j].inum = -1;
                //printf("newDblk.entries[j].inum: %d\n",newDblk.entries[j].inum);
            }
            write_block(new_blk_num,(void *)&newDblk);

            dir_block_t temp;
            read_block(5,(void *)&temp);
            //for(int j=0; j<128; j++){
            // if (temp.entries[j].inum != -1){
            //      //printf("temp.entries[i].name: %s\n",temp.entries[j].name);
            //      printf("temp.entries[i].inum  %d\n",temp.entries[j].inum );
            //     }
            // }
            return 0;
        }
        else{
            itable.inodes[newInode].size = sizeof(UFS_BLOCK_SIZE);
            write_block(s->inode_bitmap_addr,(void *)&itable);
            return 0;
        }

        // re read whole directory entries
        
        return 0;
    }
    return 0;
}

// helper functions
int parse_command(char *message, char **argList, int *argSize)
{
    char *pch;
    pch = strtok(message, ":");
    while (pch != NULL)
    {
        argList[(*argSize)++] = strdup(pch);
        pch = strtok(NULL, ":");
    }
    strtok(message, ":");
    return 0;
}

int execCommand(char **argList, int *argSize)
{
    int cmdCode = atoi(argList[0]);
    switch (cmdCode)
    {
    case 1:
        mfs_lookup(argList, argSize);
        break;
    case 2:
        mfs_stat(argList, argSize);
        break;
    case 3:
        mfs_write(argList, argSize);
        break;
    case 4:
        mfs_read(argList, argSize);
        break;
    case 5:
        mfs_creat(argList, argSize);
        break;
    default:
        perror("command not found");
        return -1;
    }
    return 0;
}

void freeAllArgs(char ** temp, int *size){
    for(int i=0; i<*size; i++)
        free(temp[i]);
    *size = 0;
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        perror("Too few arguements");
        exit(1);
    }

    char *argList[MAXLIST];
    int argSize = 0;

    int portnum = atoi(argv[1]);
    // Initialize diskimage
    char *diskImg = argv[2];
    fd = open(diskImg, O_RDWR);
    assert(fd > -1);

    struct stat sbuf;
    int rc = fstat(fd, &sbuf);
    assert(rc > -1);

    int image_size = (int)sbuf.st_size;

    void *image = mmap(NULL, image_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    assert(image != MAP_FAILED);
    s = (super_t *)image;
    printf("inode bitmap address %d [len %d]\n", s->inode_bitmap_addr, s->inode_bitmap_len);
    printf(" data bitmap address %d [len %d]\n", s->data_bitmap_addr, s->data_bitmap_len);
    printf(" data region address %d [len %d]\n", s->data_region_addr, s->data_region_len);
    printf(" inode region address %d [len %d]\n", s->inode_region_addr, s->inode_region_len);

    int sd = UDP_Open(portnum);
    assert(sd > -1);
    while (1)
    {
        struct sockaddr_in addr;
        char message[BUFFER_SIZE];
        printf("server:: waiting...\n");
        int rc = UDP_Read(sd, &addr, message, BUFFER_SIZE);
        printf("server:: read message [size:%d contents:(%s)]\n", rc, message);
        if (rc > 0)
        {
            parse_command(message, argList, &argSize);
            execCommand(argList, &argSize);
            char reply[BUFFER_SIZE];
            sprintf(reply, "goodbye world");
            rc = UDP_Write(sd, &addr, reply, BUFFER_SIZE);
            printf("server:: reply\n");
            freeAllArgs(argList,&argSize);
        }
    }
    return 0;
}
