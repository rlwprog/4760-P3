#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/msg.h>
#include <getopt.h>
#include <string.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SHMKEY	86868             /* Parent and child agree on common key.*/

#define PERMS (mode_t)(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define FLAGS (O_CREAT | O_EXCL)

typedef struct {
	int seconds;
	int nanosecs;
	int pid;
	double shmMsg;
} messageStruct;

int main (int argc, char *argv[]) {

	int pid = getpid();
    sem_t *sem;
    srand(time(NULL)*(pid^2) % 10000);
    int randNum = rand() % 1000000 + 1;
    int endtimeSec = 0;
    int endtimeNano = 0;
    int doneLooping = 0;

	static messageStruct *clock;

	sem = sem_open("clockSem",O_CREAT, PERMS, 0);

    printf("Child process enterred: %d rand: %d\n", pid, randNum);


	int shmid = shmget(SHMKEY, sizeof(messageStruct), 0666 | IPC_CREAT);

	clock = (messageStruct *)shmat(shmid, NULL, 0);
	

	while(!doneLooping){
		sem_wait(sem);
		if(endtimeSec == 0 && endtimeNano == 0){
			endtimeNano = randNum + clock->nanosecs;
	        endtimeSec = clock->seconds;
	        if (endtimeNano >= 1000000000){
	            endtimeSec += 1;
	        }
	        if (endtimeSec >= 2){
	            endtimeSec = 2;
	            endtimeNano = 0;
	        } else {
	            endtimeNano = endtimeNano % 1000000000;
	        }
        }  
         //printf("Child %d end time: %d : %d\n", pid, endtimeSec, endtimeNano);
            
		//printf("Child %d reads seconds: %d\n", pid, clock->seconds);
		//printf("Child %d reads nanoseconds: %d\n", pid, clock->nanosecs);
	 	if (clock->pid == 0){
            if(endtimeSec < clock->seconds){
            	clock->pid = pid;
            	clock->shmMsg = (double)endtimeSec + ((double)endtimeNano / 1000000000);
                doneLooping = 1;
                // printf("Child %d complete!\n", pid);
            } else if (endtimeSec <= clock->seconds && endtimeNano <= clock->nanosecs){
            	clock->pid = pid;
            	clock->shmMsg = (double)endtimeSec + ((double)endtimeNano / 1000000000);
                doneLooping = 1;
                // printf("Child %d complete!\n", pid);

            }

        }

	 	sem_post(sem);
	}

 	shmdt(clock);


 	return 1;
}
