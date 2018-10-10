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

	static messageStruct *clock;

	sem = sem_open("clockSem",O_CREAT, PERMS, 0);

	printf("Child process enterred: %d\n", pid);


	int shmid = shmget(SHMKEY, sizeof(messageStruct), 0666 | IPC_CREAT);

	clock = (messageStruct *)shmat(shmid, NULL, 0);
	
	sem_wait(sem);
	printf("Child %d reads seconds: %d\n", pid, clock->seconds);
	clock->seconds += 100;
	printf("Child %d reads seconds again: %d\n", pid, clock->seconds);
 	if (clock->pid == 0){
 		clock->pid = pid;
 	}

 	sem_post(sem);

 	shmdt(clock);


 	return 0;
}