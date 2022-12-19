#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
// #include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "udp.h"
#include "ufs.h"
#include "mfs.h"


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

int read_block_from(int blk, void*block,int off, int nbytes){
    off_t offset = blk*UFS_BLOCK_SIZE;
    lseek(fd,offset,SEEK_SET);
    //int fd2=fd+off;
    //lseek(fd2,off,SEEK_SET);
    int size = read(fd, block, nbytes);
    return 0;
}
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

int write_block_from(int blk, void*block,int off, int nbytes){
    off_t offset = blk*UFS_BLOCK_SIZE;
    printf("offset %ld\n",offset);
    lseek(fd,offset,SEEK_SET);
    //int fd2 = fd;
    //lseek(fd2,off,SEEK_SET);
    int size = write(fd, block, nbytes);
    fsync(fd);
    return 0;
}

int write_block(int blk, void *block){
    off_t offset = blk * UFS_BLOCK_SIZE;
    lseek(fd,offset,SEEK_SET);
    int size = write(fd, block, UFS_BLOCK_SIZE);
    fsync(fd);
    if (size != UFS_BLOCK_SIZE)
    {
        return -1;
    }
    return 0;
}

int reclaim_block(int addr, int blockNum){
    bitmap_t bitmap;
    read_block(addr, (void *)&bitmap);
    int byte = blockNum / 32;
    int bit = blockNum % 32;
    int mask = 0x80000000>> bit;
    bitmap.bits[byte] ^= mask;
    write_block(addr, (void *)&bitmap);
    return 0;
}
int get_inodeBLock(int inum,MFS_Stat_t *m){
    int blk = (inum / INUM_PER_BLOCK) + s->inode_region_addr;
    int inum_in_blk = inum % INUM_PER_BLOCK;
    printf("\ninum in blk %d",inum_in_blk);
    // Read blk containing inode
    inode_block iBlock;
    int rc=read_block(blk, (void *)&iBlock);
    if(rc>=0){
    m->type=iBlock.inodes[inum_in_blk].type;
    m->size=iBlock.inodes[inum_in_blk].size;
    return iBlock.inodes[inum_in_blk].direct[0];
    }
    else 
        return -1;
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
    for (int i = *cur_entry; i < DIRECT_PTRS; i++)
    {
       // if inode is not of desired type -1 if can be of either type
        if (itype != -1 && iBlock.inodes[pinum_in_blk].type != itype)
            return -1;
        if (iBlock.inodes[pinum_in_blk].direct[i] == -1)
            return -1;
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
                    return (byte * 1024) + bit;
                }
            }
        }
    }
    return -1;
}

// mfs Code
int mfs_lookup(char **argList, int *argSize)
{

    if (*argSize < 3)
        return -1;

    int pinum = atoi(argList[1]);
    int dataBlk = 0, cur_dir_entry = 0;
    char *name = argList[2];
    dir_block_t dEntries;
    while (1)
    {
        dataBlk = get_datablock(pinum, &cur_dir_entry, 0);
        if (dataBlk == -1)
            return -1;
        read_block(dataBlk, (void *)&dEntries);
        for (int i = 0; i < 128; i++)
        {  
            if ((dEntries.entries[i].inum != -1) && (strcmp(dEntries.entries[i].name, name) == 0))
            {
                return dEntries.entries[i].inum;
            }
        }
        printf("Did not find file\n");
        return -1;
    }
    return -1;
}

int mfs_stat(char **argList, int *argSize)
{
    if(*argSize<1)
    {
        return -1;
    }
    int inum = atoi(argList[1]);
    MFS_Stat_t *m = (MFS_Stat_t *)argList[2];
    int rc = get_inodeBLock(inum, m);
    printf("*****\nStat Function: \n");
    printf("type:%d ",m->type);
    printf("size:%d\n",m->size);
    argList[2] = (char *)m;
    return rc;

    // }
    return 0;
}

int mfs_write(char **argList, int *argSize)
{
    int inum,offset,nbytes;
    if(*argSize!=5){
        return -1;
    }
    inum = atoi(argList[1]);
    char buffer[4096];
    memcpy(buffer,argList[2],4096);
    printf("buffer has %s\n",buffer);
    MFS_Stat_t stat;
    int dblk=get_inodeBLock(inum,&stat);
    printf("data block of file: %d\n", dblk);
    offset=atoi(argList[3]);
    nbytes=atoi(argList[4]); 
    printf("n bytes %d", nbytes);
    // if(offset >= 30*4096)
    //     return -1;
    if(stat.type == UFS_DIRECTORY)
        return -1;
    write_block_from(dblk,(void *)buffer,offset,nbytes);

    //update size of file when written
    int blk = (inum / INUM_PER_BLOCK) + s->inode_region_addr;
    int inum_in_blk = inum % INUM_PER_BLOCK;
    // Read blk containing inode
    inode_block iBlock;
    int rc=read_block(blk, (void *)&iBlock);
    iBlock.inodes[inum_in_blk].size+=nbytes;
    write_block(blk,(void *)&iBlock);
    return 0;
}

int mfs_read(char **argList, int *argSize)
{
    int inum,offset,nbytes;
    if(*argSize!=4){
        return -1;
    }
    if(offset % sizeof(dir_ent_t)!=0)
        return -1;
    else{
        inum=atoi(argList[0]);
        offset=atoi(argList[2]);
        nbytes=atoi(argList[3]);
        char *buffer = argList[1];
        MFS_Stat_t stat;
        get_inodeBLock(inum,&stat);
        if(stat.size < nbytes)
            return -1;
        int dblk =get_datablock(inum,0,-1);
        read_block_from(dblk,(void *)buffer,offset,nbytes);
    }
    return 0;
}

int mfs_creat(char **argList, int *argSize)
{
    if (*argSize < 4)
        return -1;

    int pinum = atoi(argList[1]);
    int type = atoi(argList[2]);
    // check if inode is of type directory
    MFS_Stat_t stats;
    get_inodeBLock(pinum,&stats);
    if (stats.type == 1)
        return -1;
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
                return 0;
            }
        }

        if (strlen(name) > MAX_FNAME)
            return -1;

        int new_blk_num = get_free_bitmap(s->data_bitmap_addr);
        new_blk_num += s->data_region_addr;
        // get inode number
        int newInode = get_free_bitmap(s->inode_bitmap_addr);
        //update current directory
        for (i = 0; i < DIRECTORY_ENTRIES; i++)
        {
            if(dEntries.entries[i].inum!= -1)
                continue;
            dEntries.entries[i].inum = newInode;
            strcpy(dEntries.entries[i].name,name);
            write_block(parentBlk,(void *)&dEntries);
            break;
        }
        
        if(i==DIRECTORY_ENTRIES)
        {
            return -1;
        }

        //update inum data
        inode_block itable;
        read_block(s->inode_region_addr,(void *)&itable);
        itable.inodes[newInode].type = type;
        printf("\nitable.inodes[newInode].type %d\n",itable.inodes[newInode].type);
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
            }
            write_block(new_blk_num,(void *)&newDblk);
            return 0;
        }
        else{
            itable.inodes[newInode].size = 0;
            write_block(s->inode_region_addr,(void *)&itable);
            return 0;
        }
        return 0;
    }
    return 0;
}
int mfs_unlink(char **argList, int *argSize){
    if(*argSize<3){
        return -1;
    }else{
            int pinum=atoi(argList[1]);
            char *name = argList[2];
            if(pinum<0){
                return -1;
            }
            int inum=mfs_lookup(argList,argSize);
            if(inum==-1){
                return -1;
            }
        MFS_Stat_t stat;
        dir_block_t dEntries;
        int cur_dir_entry,dataBlk;
        get_inodeBLock(inum,&stat);
        printf("Inode num%d\n",inum);
        cur_dir_entry = 0;
        
        if(stat.type == 0){
            dataBlk = get_datablock(inum,&cur_dir_entry,-1);
            read_block(dataBlk, (void *)&dEntries);
            for (int i = 2; i < 128; i++)
            {  
                if ((dEntries.entries[i].inum != -1))
                {
                printf("Dir not empty\n");
                return -1;
                }
            }
        //If Directory Empty reclain block
        }
        reclaim_block(s->data_bitmap_addr, dataBlk);
        reclaim_block(s->inode_bitmap_addr,inum);
        cur_dir_entry = 0;
        dataBlk = get_datablock(pinum,&cur_dir_entry,0);
        read_block(dataBlk, (void *)&dEntries);
        for (int i = 0; i < 128; i++)
        {  
            if ((dEntries.entries[i].inum != -1) && (strcmp(dEntries.entries[i].name, name) == 0))
            {
                printf("found file in unlink\n");
                dEntries.entries[i].inum=-1;
                dEntries.entries[i].name[0] = '\0';
                write_block(dataBlk,(void *)&dEntries);
                break;
            }
        }
    }
    return 0;
}
int mfs_shutdown(){
    fsync(fd);
    exit(0);
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

int execCommand(char **argList, int *argSize,int sd,struct sockaddr_in *addr)
{
    int cmdCode = atoi(argList[0]);
    int result=-2,rc=-1;
    char res[BUFFER_SIZE]={0};
    char str_int[10];
    switch (cmdCode)
    {
    case 1:
        result=mfs_lookup(argList, argSize);
        sprintf(str_int,"%d",result);
        strcat(res,str_int);
        rc=UDP_Write(sd,addr,res,BUFFER_SIZE);
        break;
    case 2:
        result=mfs_stat(argList, argSize);
        MFS_Stat_t *m = (MFS_Stat_t *)argList[2];
        printf("m->type %d m->size %d", m->type,m->size);
        sprintf(str_int,"%d",m->type);
        printf("sprintf %s",str_int);
        strcat(res,str_int);
        strcat(res,":");
        sprintf(str_int,"%d",m->size);
        strcat(res,str_int);
        printf("res: %s",res);
        rc=UDP_Write(sd,addr,res,BUFFER_SIZE);
        break;
    case 3:
        result=mfs_write(argList, argSize);
        sprintf(str_int,"%d",result);
        strcat(res,str_int);
        rc=UDP_Write(sd,addr,res,BUFFER_SIZE);
        break;
    case 4:
        mfs_read(argList, argSize);
        memcpy(res,argList[2],BUFFER_SIZE);
        rc=UDP_Write(sd,addr,res,BUFFER_SIZE);
        break;
    case 5:
        result=mfs_creat(argList, argSize);
        sprintf(str_int,"%d",result);
        strcat(res,str_int);
        rc=UDP_Write(sd,addr,res,BUFFER_SIZE);
        break;
    case 6:
        result=mfs_unlink(argList, argSize);
        sprintf(str_int,"%d",result);
        strcat(res,str_int);
        rc=UDP_Write(sd,addr,res,BUFFER_SIZE);
        break;
    case 7:
        result=mfs_shutdown();
        sprintf(str_int,"%d",result);
        strcat(res,str_int);
        rc=UDP_Write(sd,addr,res,BUFFER_SIZE);
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
        int rc = UDP_Read(sd, &addr, message, BUFFER_SIZE);
        if (rc > 0)
        {
            parse_command(message, argList, &argSize);
            execCommand(argList, &argSize,sd,&addr);
            freeAllArgs(argList,&argSize);
        }
    }
    return 0;
}
