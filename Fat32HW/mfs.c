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
#include <stdint.h>

#define MAX_NUM_ARGUMENTS 3

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

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
int8_t  BPD_NumFATs;
int32_t BPD_FATSz32;

int main()
{
  FILE *fp;
  int close = 1;
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
      if ( !strcmp(token[0],"open") )
			{
        if ( fp )
        {
          perror("Error: File system image already open.");
          break;
        }

        if ( sizeof(token[1]) > 100 )
        {
          perror("Error: File name too large.");
          break;
        }

        if ( !fopen(fp, token[1]) )
        {
          perror("Error: File system image no found.");
          break;
        }

        fseek(fp, 11, SEEK_SET);
        fread(&BPB_BytsPerSec, 2, 1, fp);

        fseek(fp, 13, SEEK_SET);
        fread(&BPD_SecPerClus, 1, 1, fp);

        fseek(fp, 14, SEEK_SET);
        fread(&BPB_RsvdSecCnt, 2, 1, fp);
        
        fseek(fp, 16, SEEK_SET);
        fread(&BPD_NumFATs, 1, 1, fp);

        fseek(fp, 36, SEEK_SET);
        fread(&BPD_FATSz32, 4, 1, fp);
			}
      else if ( !strcmp(token[0],"close") )
      {
        if ( fp )
        {
          fclose(fp);
          fp = NULL;
          break;
        }
    
        perror("Error: File system is not open.");
      }
      else if ( !strcmp(token[0],"info") )
      {
        if( !fp )
          perror("Error: File system image must be opened first.");

        printf("%d %x\n", BPB_BytsPerSec, BPB_BytsPerSec);
        printf("%d %x\n", BPD_SecPerClus, BPD_SecPerClus);
        printf("%d %x\n", BPB_RsvdSecCnt, BPB_RsvdSecCnt);
        printf("%d %x\n", BPD_NumFATs   , BPD_NumFATs   );
        printf("%d %x\n", BPD_FATSz32   , BPD_FATSz32   );
      }
      else if ( !strcmp(token[0],"stat") )
      {
        if( !fp )
          perror("Error: File system image must be opened first.");
      }
      else if ( !strcmp(token[0],"cd") )
      {
        if( !fp )
          perror("Error: File system image must be opened first.");
      }
      else if ( !strcmp(token[0],"ls") )
      {
        if( !fp )
          perror("Error: File system image must be opened first.");
      }
      else if ( !strcmp(token[0],"get") )
      {
        if( !fp )
          perror("Error: File system image must be opened first.");
      }
      else if ( !strcmp(token[0],"read") )
      {
        if( !fp )
          perror("Error: File system image must be opened first.");
      }
    }

    free( working_root );

  }
  return 0;
}
