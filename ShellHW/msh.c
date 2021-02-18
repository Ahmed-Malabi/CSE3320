// The MIT License (MIT)
// 
// Copyright (c) 2016, 2017, 2021 Trevor Bakker 
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
// 7f704d5f-9811-4b91-a918-57c1bb646b70
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

#define WHITESPACE " \t\n"			// We want to split our command line up into tokens
                         			// so we need to define what delimits our tokens.
                         			// In this case  white space
                            		// will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255		// The maximum command-line size

#define MAX_NUM_ARGUMENTS 11     // Mav shell only supports 11 arguments

#define HISTORY 15					// The size of history including null terminator

int main()
{
	int i, cmdCounter, pidCounter;
	int pidHistory[HISTORY] = {};

	char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );
	char** cmdHistory;
	cmdHistory = malloc((HISTORY + 1) * sizeof(char*));
	for ( i = 0; i < HISTORY; i++ )
		cmdHistory[i] = malloc(MAX_COMMAND_SIZE);
		

	while( 1 )
	{
		// Print out the msh prompt
		printf ("msh> ");

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
		char *argument_ptr;                                         
				                                               
		char *working_str  = strdup( cmd_str );                

		// we are going to move the working_str pointer so
		// keep track of its original value so we can deallocate
		// the correct amount at the end
		char *working_root = working_str;

		// Tokenize the input strings with whitespace used as the delimiter
		while ( ( (argument_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
				  (token_count<MAX_NUM_ARGUMENTS))
		{
			token[token_count] = strndup( argument_ptr, MAX_COMMAND_SIZE );
			if ( strlen( token[token_count] ) == 0 )
			{
				token[token_count] = NULL;
			}
				token_count++;
		}
		
		if ( token[0] != NULL )
		{
			// add command to the history
			
			if ( cmdCounter < HISTORY ) )
			{
				strcpy(cmdHistory[cmdCounter], token[0]);
				cmdCounter++;;
			}
			else
			{
				// rotate the history up the list if history is full
				for ( i = 0; i < (HISTORY - 1); i++ )
				{
					strcpy(cmdHistory[i], cmdHistory[i+1]);
				}
				strcpy(cmdHistory[i], token[0]);
			}
			
			// check user input to determine if it needs to be an
			// inbuilt function or if we can just delegate to execpv

		 	if ( !strcmp(token[0],"quit") || !strcmp(token[0],"exit") )
			{
				exit( 0 );
			}
			else if ( !strcmp(token[0],"cd") )
			{
				if( chdir(token[1]) )
				{
					perror(token[1]);
				}
			}
			else if ( !strcmp(token[0],"history") )
			{
				i = 0;
				while ( strcmp(cmdHistory[i],"") && i < HISTORY)
				{
					printf("%d: %s\n", i, cmdHistory[i]);
					i++;
				}
			}
			else if ( !strcmp(token[0],"showpid") )
			{
				i = 0;
				while ( pidHistory[i] != 0 && i < HISTORY)
					printf("%d: %d\n", i, pidHistory[i++]);
			}
			else
			{
				pid_t pid = fork();
				if ( pid == 0 )
				{
					// let exec handle user input and errors
					int ret = execvp( token[0], &token[0]);
					if ( ret == -1 )
						perror("exec failed: ");
				}
				else
				{
					int status;
					
					// add pid to the history
					// reused from cmdHisory
			
					if ( pidCounter < HISTORY )
					{
						pidHistory[pidCounter] = pid;
						pidCounter++; 
					}
					else
					{
						// rotate the history up the list if history is full
						for ( i = 0; i < HISTORY - 1; i++ )
						{
							pidHistory[i] = pidHistory[i+1];
						}
						pidHistory[i] = pid;
					}
					
					wait( &status );
				}
			}
		}
		free( working_root );
	}
	return 0;
	
}
