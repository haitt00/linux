// #include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/msg.h>
#include <limits.h>


#define KEYFILE_PATH "28+^E$"
#define PROJ_CHAR    'R'
#define MSGQ_OK 0
#define MSGQ_NG -1

int           msqid; //message queue

int nextID = 1;
int main(){
  	printf("accOrderCtrl:	start\n");

	//Prepare message queue
	key_t keyval = ftok( KEYFILE_PATH,(int)PROJ_CHAR);
    msqid = msgget(keyval, IPC_CREAT|0660);
    if(msqid == MSGQ_NG){
        perror("msgget:opeProc");
        exit(1);
    }
    pid_t pid;
	while(1){
		//receives from MQ: ACC - id(mtype), pid(mtext)
		struct msgbuff{
            long mtype;
            pid_t mtext;
        }message;

        if((msgrcv(msqid, &message, sizeof(message.mtext), nextID, 0)) ==
           MSGQ_NG){
            perror("msgrcv:opeProc");
            exit(1);
        }
        pid = message.mtext;
  		printf("accOrderCtrl:	receive ACC from MQ(id=%ld, pid=%ld)\n", message.mtype, (long) message.mtext);

		//send signal to pid
		kill(pid, SIGUSR1);
  		printf("accOrderCtrl:	send signal to %ld\n", (long) pid);

		//receives from MQ: FIN
		if((msgrcv(msqid, &message, sizeof(message.mtext), LONG_MAX - nextID, 0)) ==
           MSGQ_NG){
            perror("msgrcv:opeProc");
            exit(1);
        }
  		printf("accOrderCtrl:	receive FIN from MQ %ld\n", (long) message.mtext);

		//update id
		nextID++;
		if(nextID > 3) nextID = 1;
  		printf("accOrderCtrl:	nextID = %d\n", nextID);

	}
  	printf("accOrderCtrl:	end\n");
	return 0;

}