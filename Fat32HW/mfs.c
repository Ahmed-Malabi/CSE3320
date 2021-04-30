// The MIT License (MIT)
// 
// Copyright (c) 2020 Trevor Bakker 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <stdint.h>

#define MAX_NUM_ARGUMENTS 3

#define WHITESPACE " \t\n"       // We want to split our command line up into tokens
                                 // so we need to define what delimits our tokens.
                                 // In this case  white space
                                 // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255     // The maximum command-line size

FILE *fp = NULL;
char expanded[12];

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
int8_t  BPB_SecPerClus;
int16_t BPB_RsvdSecCnt;
int8_t  BPB_NumFATs;
int32_t BPB_FATSz32;

int LBAToOffset(int32_t sector)
{
   return ((sector - 2) * BPB_BytsPerSec) + (BPB_BytsPerSec * BPB_RsvdSecCnt)
         + (BPB_NumFATs * BPB_FATSz32 * BPB_BytsPerSec);
}

int16_t NextLB(uint32_t sector)
{
   uint32_t FATAddress = (BPB_BytsPerSec * BPB_RsvdSecCnt) + (sector * 4);
   int16_t val;
   fseek(fp, FATAddress, SEEK_SET);
   fread(&val, 2, 1, fp);
   return val;
}

void expand(char* toExpand)
{
   char *token = strtok( toExpand, "." );
   memset( expanded, ' ', 12 );

   strncpy( expanded, token, strlen( token ) );

   token = strtok( NULL, "." );

   if( token )
   {
      strncpy( (char*)(expanded+8), token, strlen(token ) );
   }

   expanded[11] = '\0';

   int i;
   for( i = 0; i < 11; i++ )
   {
      expanded[i] = toupper( expanded[i] );
   }
}

int main()
{
   char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );

   while( 1 )
   {
      // Print out the mfs prompt
      printf ("mfs> ");

      // Read the command from the commandline.  The
      // maximum command that will be read is MAX_COMMAND_SIZE
      // This while command will wait here until the user
      // inputs something since fgets returns NULL when there
      // is no input
      while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

      /* Parse input */
      char *token[MAX_NUM_ARGUMENTS];

      int   token_count = 0;                                 
                                                   
      // Pointer to point to the token
      // parsed by strsep
      char *arg_ptr;                                         
                                                   
      char *working_str  = strdup( cmd_str );                

      // we are going to move the working_str pointer so
      // keep track of its original value so we can deallocate
      // the correct amount at the end
      char *working_root = working_str;

      // Tokenize the input stringswith whitespace used as the delimiter
      while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
               (token_count<MAX_NUM_ARGUMENTS))
      {
         token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
         if( strlen( token[token_count] ) == 0 )
         {
               token[token_count] = NULL;
         }
         token_count++;
      }

      // Now print the tokenized input as a debug check
      // \TODO Remove this code and replace with your FAT32 functionality

      if ( token[0] != NULL )
      {
         if ( !strcmp(token[0],"quit") )
         {
            break;
         }
         else if ( !strcmp(token[0],"open") )
         {
            if ( fp )
            {
               printf("Error: File system image already open.\n");
            }
            else
            {
               fp = fopen(token[1], "r");
               if ( fp )
               {
                  fseek(fp, 11, SEEK_SET);
                  fread(&BPB_BytsPerSec, 2, 1, fp);

                  fseek(fp, 13, SEEK_SET);
                  fread(&BPB_SecPerClus, 1, 1, fp);

                  fseek(fp, 14, SEEK_SET);
                  fread(&BPB_RsvdSecCnt, 2, 1, fp);

                  fseek(fp, 16, SEEK_SET);
                  fread(&BPB_NumFATs,    1, 1, fp);

                  fseek(fp, 36, SEEK_SET);
                  fread(&BPB_FATSz32,    4, 1, fp);

                  int root =  (BPB_NumFATs*BPB_FATSz32*BPB_BytsPerSec)+
                              (BPB_RsvdSecCnt*BPB_BytsPerSec);
                  fseek(fp, root, SEEK_SET);
                  fread( &dir[0], sizeof(struct DirectoryEntry), 16, fp);
               }
               else
               {
                  printf("Error: File system image no found.\n");
               }
            }
         }
         else if ( !strcmp(token[0],"close") )
         {
            if ( fp )
            {
               fclose(fp);
               fp = NULL;
            }
            else
            {
               printf("Error: File system is not open.\n");
            }
         }
         else if ( !strcmp(token[0],"info") )
         {
            if( !fp )
            {
               printf("Error: File system image must be opened first.\n");
            }
            else
            {
               printf("BPB_BytsPerSec:\t%-6d %-4x\n", BPB_BytsPerSec, BPB_BytsPerSec);
               printf("BPB_SecPerClus:\t%-6d %-4x\n", BPB_SecPerClus, BPB_SecPerClus);
               printf("BPB_RsvdSecCnt:\t%-6d %-4x\n", BPB_RsvdSecCnt, BPB_RsvdSecCnt);
               printf("BPB_NumFATs:\t%-6d %-4x\n"   , BPB_NumFATs   , BPB_NumFATs   );
               printf("BPB_FATSz32:\t%-6d %-4x\n"   , BPB_FATSz32   , BPB_FATSz32   );
            }
         }
         else if ( !strcmp(token[0],"stat") )
         {
            if( !fp )
            {
               printf("Error: File system image must be opened first.\n");
            }
            else
            {

            }
         }
         else if ( !strcmp(token[0],"cd") )
         {
            if( !fp )
            {
               printf("Error: File system image must be opened first.\n");
            }
            else
            {
               expand(token[1]);

               int i = 0;
               for(i = 0; i < 16; i++)
               {
                  if(dir[i].DIR_Name == expanded && dir[i].DIR_Attr == 16)
                  {

                  }
               }

            }
         }
         else if ( !strcmp(token[0],"ls") )
         {
            if( !fp )
            {
               printf("Error: File system image must be opened first.\n");
            }
            else if( token[1] == NULL || !strcmp(token[1],".") )
            {
               //printing contents of directory
               int i;
               for(i = 0; i < 16; i++)
               {
                  //we create a temp variable in order to add null terminator
                  //to the end of the filename
                  char filename[12];
                  strncpy(&filename[0], &dir[i].DIR_Name[0], 11);
                  filename[11] = '\0';
                  if ( dir[i].DIR_Attr == 1 || dir[i].DIR_Attr == 16 || dir[i].DIR_Attr == 32 )
                  {
                     if ( dir[i].DIR_Name[0] != 0x00 && dir[i].DIR_Name[0] != 0xe5 )
                        printf("%s\n", filename);
                  }
               }
            }
            else if( !strcmp(token[1],"..") )
            {

            }
            else
            {
               printf("Error: invalid use of ls.");
            }
         }
         else if ( !strcmp(token[0],"get") )
         {
            if( !fp )
            {
               printf("Error: File system image must be opened first.\n");
            }
            else
            {

            }
         }
         else if ( !strcmp(token[0],"read") )
         {
            if( !fp )
            {
               printf("Error: File system image must be opened first.\n");
            }
            else
            {

            }
         }
         else
         {
            printf("%s: command not found\n", token[0]);
         }
      }

      free( working_root );

   }
   return 0;
}
