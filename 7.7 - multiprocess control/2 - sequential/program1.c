#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <limits.h>
#include<time.h>

#define KEYFILE_PATH "28+^E$"
#define PROJ_CHAR    'R'
#define MSGQ_OK 0
#define MSGQ_NG -1
#define LOOP_ON 1
#define LOOP_OFF 0

int           semid; //semaphore
struct sembuf buff;

int *ptr;			//shared memory

int           msqid; //message queue

int loopFlag = LOOP_ON;

void sigHandle1(int sigNo);
void sigHandle2(int sigNo);
void sigHandle3(int sigNo);

int main(){
  	printf("memAccProcGen:	start\n");

	//Prepare shared memory
	key_t keyval = ftok( KEYFILE_PATH,(int)PROJ_CHAR);
  	int shmsize = sizeof(int) * 2;
	int       shmid;
	if ((shmid = shmget(keyval,shmsize,
	                    IPC_CREAT|0660)) == -1){
	  	perror("shmget");
	  	exit(1);
	} 
	ptr = (int*)shmat(shmid,0,0);
	if ((int*)shmat(shmid,0,0)
	                         == (int*)-1)
	{
	  	perror("shmat");
	  	exit(1);
	}

	//Prepare semaphore
  	semid = semget(keyval, 1, IPC_CREAT|0660);
	//Sembuf structure setting for semaphore operation
	buff.sem_num = (ushort)0;
	buff.sem_op = (short)1;
	buff.sem_flg=(short)0;
	semop(semid, &buff, 1);

	//Prepare message queue
    msqid = msgget(keyval, IPC_CREAT|0660);
    if(msqid == MSGQ_NG){
        perror("msgget:opeProc");
        exit(1);
    }

	//Create 3 children process
  	pid_t pid = 0;
	int i;
	for(i = 1; i <= 3; i++){
		pid = fork();
		if(pid == 0) break;
	}
	// printf("%d: pid: %ld\n", i, pid);

	if(pid != 0) printf("memAccProcGen:	fork\n");
	
  	int status = 0;
	int r;
    switch (i){
      	case -1:
      	  	exit(1);

      	case 1:
      		printf("memAccProc1:	start\n");
  			signal(SIGUSR1, sigHandle1);
      		printf("memAccProc1:	register signal handler\n");
	      	srand(time(NULL) + 1);
      		while(1){
	      		
	      		//Sleep
	      		r = rand() % 5 + 1;
	      		// r = 30;
      			printf("memAccProc1:	sleep %d seconds\n", r);
	      		// Signal handler function registration
	      		sleep(r); 

	      		//Send access request to MQ
	      		struct msgbuff{
		            long mtype;
		            pid_t mtext;
		        }message;
		        message.mtype = 1;
		        message.mtext = getpid();
	      		if(msgsnd(msqid, &message, sizeof(message.mtext), 0) == MSGQ_NG){
		            perror("msgsnd:opeProc");
		            exit(1);
		        }
      			printf("memAccProc1:	send ACC to MQ\n");

		        //Wait for signal
		        loopFlag = LOOP_ON;
		        while(loopFlag == LOOP_ON);

		        //Send finish request to MQ
		        message.mtype = LONG_MAX - 1;
		        if(msgsnd(msqid, &message, sizeof(message.mtext), 0) == MSGQ_NG){
		            perror("msgsnd:opeProc");
		            exit(1);
		        }
      			printf("memAccProc1:	send FIN to MQ\n");

      		}
      		
  			printf("memAccProc1:	end\n");
      		exit(1);

      	case 2:
      		printf("memAccProc2:	start\n");
  			signal(SIGUSR1, sigHandle2);
      		printf("memAccProc2:	register signal handler\n");
	      	srand(time(NULL) + 2);
      		while(1){
	      		
	      		//Sleep
	      		r = rand() % 5 + 1;
	      		// r = 1000;
	      		// Signal handler function registration
      			printf("memAccProc2:	sleep %d seconds\n", r);
	      		sleep(r); 

	      		//Send access request to MQ
	      		struct msgbuff{
		            long mtype;
		            pid_t mtext;
		        }message;
		        message.mtype = 2;
		        message.mtext = getpid();
	      		if(msgsnd(msqid, &message, sizeof(message.mtext), 0) == MSGQ_NG){
		            perror("msgsnd:opeProc");
		            exit(1);
		        }
      			printf("memAccProc2:	send ACC to MQ\n");

		        //Wait for signal
		        loopFlag = LOOP_ON;
		        while(loopFlag == LOOP_ON);

		        //Send finish request to MQ
		        message.mtype = LONG_MAX - 2;
		        if(msgsnd(msqid, &message, sizeof(message.mtext), 0) == MSGQ_NG){
		            perror("msgsnd:opeProc");
		            exit(1);
		        }
      			printf("memAccProc2:	send FIN to MQ\n");

      		}
  			printf("memAccProc2:	end\n");
      		exit(1);

     	case 3:
     		printf("memAccProc3:	start\n");
  			signal(SIGUSR1, sigHandle3);
      		printf("memAccProc3:	register signal handler\n");
	      	srand(time(NULL) + 3);
      		while(1){
	      		
	      		//Sleep
	      		r = rand() % 5 + 1;
	      		// r = 1000;
	      		// Signal handler function registration
      			printf("memAccProc3:	sleep %d seconds\n", r);
	      		sleep(r); 

	      		//Send access request to MQ
	      		struct msgbuff{
		            long mtype;
		            pid_t mtext;
		        }message;
		        message.mtype = 3;
		        message.mtext = getpid();
	      		if(msgsnd(msqid, &message, sizeof(message.mtext), 0) == MSGQ_NG){
		            perror("msgsnd:opeProc");
		            exit(1);
		        }
      			printf("memAccProc3:	send ACC to MQ\n");

		        //Wait for signal
		        loopFlag = LOOP_ON;
		        while(loopFlag == LOOP_ON);

		        //Send finish request to MQ
		        message.mtype = LONG_MAX - 3;
		        if(msgsnd(msqid, &message, sizeof(message.mtext), 0) == MSGQ_NG){
		            perror("msgsnd:opeProc");
		            exit(1);
		        }
      			printf("memAccProc3:	send FIN to MQ\n");

      		}
  			printf("memAccProc3:	end\n");
      		exit(1);

      	default: //memAccProcGen
  			for(int j = 0; j < 3; j++){
  				pid=wait(&status);
      			printf("pid=%d,status=%d\n", pid, status);
      			printf("WIFEXITED: %d\n", WIFEXITED(status));
      			printf("WIFSIGNALED: %d\n", WIFSIGNALED(status));
      			printf("WIFSTOPPED: %d\n", WIFSTOPPED(status));

  			}
  			printf("memAccProcGen:	end\n");
      		exit(1);
	}

}
void sigHandle1(int sigNo)
{
  	// printf("Signal No. %d\n", sigNo);
  	printf("sigHandle1:		start\n");
  
  	//Lock semaphore
	buff.sem_op = (short)-1;
	semop(semid, &buff, 1);
  	printf("sigHandle1:		lock sem\n");


  	//Write to shemVal
	*ptr = 10;
	*(ptr+1) = 0; 
  	printf("sigHandle1:		access shemVal\n");

  	//Unlock semaphore
  	buff.sem_op = (short)1;
	semop(semid, &buff, 1);
	printf("sigHandle1:		unlock sem\n");



	loopFlag = LOOP_OFF;
  	printf("sigHandle1:		end\n");

  	return;
}
void sigHandle2(int sigNo)
{
  	// printf("Signal No. %d\n", sigNo);
  	printf("sigHandle2:		start\n");
  
  	//Lock semaphore
	buff.sem_op = (short)-1;
	semop(semid, &buff, 1);
  	printf("sigHandle2:		lock sem\n");

  	//Write to shemVal
  	printf("sigHandle2:		100/X = %d\n",100/(*ptr));
	*ptr = 0;
	*(ptr+1) = 10; 
  	printf("sigHandle2:		access shemVal\n");
	
  	//Unlock semaphore
  	buff.sem_op = (short)1;
	semop(semid, &buff, 1);
	printf("sigHandle2:		unlock sem\n");


	loopFlag = LOOP_OFF;
  	printf("sigHandle2:		end\n");
  	return;
}
void sigHandle3(int sigNo)
{
  	// printf("Signal No. %d\n", sigNo);
  	printf("sigHandle3:		start\n");
  
  	//Lock semaphore
	buff.sem_op = (short)-1;
	semop(semid, &buff, 1);
  	printf("sigHandle3:		lock sem\n");

  	//Write to shemVal
  	printf("sigHandle3:		10000/Y = %d\n",10000/(*(ptr+1)));
	*ptr = 0;
	*(ptr+1) = 0; 
  	printf("sigHandle3:		access shemVal\n");
	
  	//Unlock semaphore
  	buff.sem_op = (short)1;
	semop(semid, &buff, 1);
	printf("sigHandle3:		unlock sem\n");


	loopFlag = LOOP_OFF;
  	printf("sigHandle3:		end\n");
  	return;
}