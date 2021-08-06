#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>

#define PIPE_READ  0
#define PIPE_WRITE 1
#define PIPE_READ_WRITE  2

#define KEYFILE_PATH "x8+^E$"
#define PROJ_CHAR    'X'
#define MSGQ_OK 0
#define MSGQ_NG -1

#define EXIT 108011135

int main(int argc, char const *argv[])
{
	
	int type = atoi(argv[1]);

	printf("inputReceiptProc:	start\n");
	printf("To exit, type 108011135\n");
	/**
	*
	Prepare pipe
	*
	*/
	int pipe_c2p[PIPE_READ_WRITE];
	memset(pipe_c2p, 0, sizeof(pipe_c2p));
	if(pipe(pipe_c2p)==-1){
		perror("inputReceiptProc pipe");
		exit(1);
	}

	/**
 	*
 	Prepare message queue
 	*
 	*/
 	key_t keyx = ftok(KEYFILE_PATH, (int)PROJ_CHAR);
 	int msqid = msgget(keyx, IPC_CREAT|0660);
 	if(msqid == MSGQ_NG){
 	  perror("msgget:inputReceiptProc");
 	  exit(1);
 	}

  	/**
	*
	Processing
	*
	*/

	//Create a child process
	pid_t pid = fork();

	switch (pid){
		case -1:
			perror("inputReceiptProc fork");
			close(pipe_c2p[PIPE_READ]);
			close(pipe_c2p[PIPE_WRITE]);
			exit(1);

		case 0: //inputReceiptProc(child)
			printf("inputReceiptProc(child):	start\n");

			close(pipe_c2p[PIPE_WRITE]);

			int readVal;
			while(1){

				//Receive input through pipe
				read(pipe_c2p[PIPE_READ], &readVal, sizeof(int));
				
				if(readVal != 0){
					printf("inputReceiptProc(child):	receive from pipe %d\n", readVal);						
					
					//Send input through message queue
					struct msgbuff{
				      long mtype;
				      int mtext;
				    }message;
				    message.mtype = type;
				    message.mtext = readVal;
				    if(msgsnd(msqid, &message, sizeof(message.mtext), 0) == MSGQ_NG){
					    perror("msgsnd:inputReceiptProc(child)");
					    exit(1);
					};
					printf("inputReceiptProc(child):	send to MQ %d\n", message.mtext);						

					//handle case EXIT
					if(readVal == EXIT){
						// Delete the message queue
						if(msgctl(msqid, IPC_RMID, NULL) == MSGQ_NG){
						  perror("msgctl:opeProc");
						}
						printf("inputReceiptProc(child):	delete MQ\n");						
						break;
					}

					//Receives operation result from MQ
					if((msgrcv(msqid, &message, sizeof(message.mtext), LONG_MAX - type, 0)) ==
				     MSGQ_NG){
					    perror("msgrcv:inputReceiptProc(child)");
						exit(1);
				  	}
				  	printf("inputReceiptProc(child):	receive from MQ %d\n", message.mtext);						
				}
			}
			printf("inputReceiptProc(child):	exit\n");						
			exit(1);

		default: //inputReceiptProc(parent)
			printf("inputReceiptProc(parent):	start\n");
			
			close(pipe_c2p[PIPE_READ]);
			
			int writeVal;
			while(1){
				
				//Accepts input though console
				scanf("%d", &writeVal);

				//Send input through pipe
				write(pipe_c2p[PIPE_WRITE], &writeVal, sizeof(writeVal));
				printf("inputReceiptProc(parent):	send to pipe %d\n", writeVal);
				if(writeVal == EXIT) break;
			}
			printf("inputReceiptProc(parent):	exit\n");						
			exit(1);
	}
}