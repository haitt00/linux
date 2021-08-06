#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include <sys/msg.h>
#include <sys/time.h>
#include <limits.h>

#define KEYFILE_PATH "x8+^E$"
#define PROJ_CHAR    'X'
#define MSGQ_OK 0
#define MSGQ_NG -1

#define EXIT 108011135

int main(int argc, char const *argv[])
{
    int type = atoi(argv[1]);
    
    printf("opeProc:    start\n");
    /**
    *
    Prepare shared memory
    *
    */
    key_t     keyval;
    int       shmsize;

    keyval = ftok( KEYFILE_PATH,(int)PROJ_CHAR);
    
    shmsize = sizeof(int) + sizeof(pthread_mutex_t);
    //Get the shared memory ID
    int       shmid;
    if ((shmid = shmget(keyval,shmsize,
                        IPC_CREAT|0660)) == -1){
        perror("shmget:opeProc");
        exit(1);
    }
    //Attach the shared memory
    int       *ptr;  
    ptr = (int*)shmat(shmid,0,0);
    if ((int*)shmat(shmid,0,0) == (int*)-1)
    {
        perror("shmat:opeProc");
        exit(1);
    }
  
    /**
    *
    Prepare mutex
    *
    */
    pthread_mutexattr_t pmattr;

    // Initialize the mutex attribute object
    pthread_mutexattr_init(&pmattr);
    // Set mutex attribute(Setting for using mutex to inter process)
    if (pthread_mutexattr_setpshared(&pmattr, PTHREAD_PROCESS_SHARED) != 0) {
      perror("pthread_mutexattr_setpshared:opeProc");
      exit(1);
    }
    // Initialize mutex
    pthread_mutex_t* pm = (pthread_mutex_t*) ((char*)ptr + sizeof(int));
    pthread_mutex_init(pm , &pmattr);

    /**
    *
    Prepare message queue
    *
    */
    key_t keyx = ftok(KEYFILE_PATH, (int)PROJ_CHAR);
    int           msqid;
    msqid = msgget(keyx, IPC_CREAT|0660);
    if(msqid == MSGQ_NG){
        perror("msgget:opeProc");
        exit(1);
    }

    /**
    *
    Processing
    *
    */

    // //Init shared memory to 0
    // // *ptr = 0;
    
    while(1){

        //Receive int val from MQ
        struct msgbuff{
            long mtype;
            int mtext;
        }message;
        if((msgrcv(msqid, &message, sizeof(message.mtext), type, 0)) ==
           MSGQ_NG){
            perror("msgrcv:opeProc");
            exit(1);
        }
        printf("opeProc:    receive from MQ %d\n", message.mtext);
        int val = message.mtext;
        if(val == EXIT){
            break;
        }

        // Lock mutex
        if (pthread_mutex_lock(pm) != 0) {
            perror("pthread_mutex_lock:opeProc");
            exit(1);
        }
        printf("opeProc:    lock mutex\n");

        //Access shemVal
        int shemVal = *ptr;
        sleep(5);
        shemVal += val; 
        *ptr = shemVal;
        printf("opeProc:    update shemVal %d\n", shemVal);

        // Unlock mutex
        if (pthread_mutex_unlock(pm) != 0) {
          perror("pthread_mutex_unlock:opeProc");
          exit(1);
        }
        printf("opeProc:    unlock mutex\n");

        //Send int shemVal to MQ
        message.mtype = LONG_MAX - type;
        message.mtext = shemVal;
        if(msgsnd(msqid, &message, sizeof(message.mtext), 0) == MSGQ_NG){
            perror("msgsnd:opeProc");
            exit(1);
        }
        printf("opeProc:    send to MQ %d (type=%ld) \n", message.mtext, message.mtype);
    }
 
    //Detach the shared memory
    if( shmdt((void*)ptr ) == -1){
        perror("shmdt:opeProc");
        exit(1);
    }
    //Delete the shared memory
    if( shmctl(shmid,IPC_RMID,0) == -1 ){
        perror("shmctl:opeProc");
        exit(1);
    }  
    printf("inputReceiptProc(child):    delete shared mem\n");                      

    return 0;
}
