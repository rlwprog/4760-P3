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

typedef struct {
	int seconds;
	int nanosecs;
} clockStruct;

int main (int argc, char *argv[]) {

	int pid = getpid();
	sem_t *sem;
	int semTest = 0;

	static clockStruct *clock;

	sem = sem_open("clockSem", 0);


	printf("Child process enterred: %d\n", pid);


	int shmid = shmget(SHMKEY, sizeof(clockStruct), 0666 | IPC_CREAT);

	printf("%d\n", shmid);

	clock = (clockStruct *)shmat(shmid, NULL, 0);
	sem_wait(sem);
	printf("Child %d reads the data before change: %d\n", pid, clock->seconds);
 	clock->seconds += 5;
 	printf("Child %d reads the data after change: %d\n", pid, clock->seconds);

 	sem_post(sem);

 	shmdt(clock);


 	return 0;
}