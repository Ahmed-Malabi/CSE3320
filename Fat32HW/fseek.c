#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

struct __attribute__((__packed__)) DirectoryEntry
{
    char DIR_Name[11];
    uint8_t DIR_Attr;
    uint8_t Unused1[8];
    uint16_t DIR_FirstClusterHigh;
    uint8_t Unused2[4];
    uint16_t DIR_FirstClusterLow;
    uint32_t DIR_FileSize;
};

struct DirectoryEntry dir[16];

int16_t BPB_BytsPerSec;
int8_t  BPD_SecPerClus;
int16_t BPB_RsvdSecCnt;

int main()
{
    FILE *fp;

    fp = fopen("fat32.img", "r");

    //Bytes Per Sector - (starts at 11, 2 bytes)
    fseek(fp,11,SEEK_SET);
    fread(&BPB_BytsPerSec,2,1,fp);
    printf("%d %x\n", BPB_BytsPerSec, BPB_BytsPerSec);

    //Sectors Per Cluster - (starts at 13, 1 byte)
    fseek(fp,13,SEEK_SET);
    fread(&BPD_SecPerClus,1,1,fp);
    printf("%d %x\n", BPD_SecPerClus, BPD_SecPerClus);

    //Reserved Sector Count - (starts at 14, 2 bytes)
    fseek(fp,14,SEEK_SET); 
    fread(&BPB_RsvdSecCnt,2,1,fp);
    printf("%d %x\n", BPB_RsvdSecCnt, BPB_RsvdSecCnt);

    //navigate to root directory (hardcoded value) and read 
    fseek(fp, 0x100400, SEEK_SET);
    fread( &dir[0], sizeof(struct DirectoryEntry), 16, fp);

    printf("\n\n\n\n");

    //printing contents of directory
    int i;
    for(i = 0; i < 16; i++)
    {
        //we create a temp variable in order to add null terminator
        //to the end of the filename
        char filename[12];
        strncpy(&filename[0], &dir[i].DIR_Name[0], 11);
        filename[11] = '\0';
        printf("%s\n", filename);
    }

    








    return 0;
}



