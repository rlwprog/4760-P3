#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>

#define SHMKEY	86868             /* Parent and child agree on common key.*/

typedef struct {
	int seconds;
	int nanosecs;
} clockStruct;

int main (int argc, char *argv[]) {

	int pid = getpid();

	static clockStruct *clock;

	printf("Child process enterred: %d\n", pid);


	int shmid = shmget(SHMKEY, sizeof(clockStruct), 0666 | IPC_CREAT);

	printf("%d\n", shmid);

	clock = (clockStruct *)shmat(shmid, NULL, 0);

	printf("Child reads the data from parent: %d\n", clock->seconds);
 	
	clock->seconds = 5;

	printf("Child reads the data after change: %d\n", clock->seconds);


 	shmdt(clock);


 	return 0;
}