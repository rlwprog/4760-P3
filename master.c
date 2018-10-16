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

#define SHMKEY	86868            /* Parent and child agree on common key.*/
#define SEMKEY 98989

#define PERMS (mode_t)(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define FLAGS (O_CREAT | O_EXCL)

static volatile sig_atomic_t doneflag = 0;

static void setdoneflag(int signo){
	doneflag = 1;
}


typedef struct {
	int seconds;
	int nanosecs;
	int pid;
	double shmMsg;
} messageStruct;

int main (int argc, char *argv[]){

	int opt = 0;
	int currentProcesses = 0;
	int processCount = 0;
	int maxProcesses = 5;
	int maxTotal = 100;
	int killTime = 2;
	char * fileName;
	sem_t *sem;
	int childPid;
	pid_t waitpid;
    int waitstatus = 0;

    FILE * fileOut;

	struct sigaction act;
	static messageStruct *clock;

	int timeLimit = 2;
	
	//set up handler for ctrl-C
	act.sa_handler = setdoneflag;
	act.sa_flags = 0;

	if ((sigemptyset(&act.sa_mask) == -1) || (sigaction(SIGINT, &act, NULL) == -1)) {
		perror("Failed to set SIGINT handler");
		return 1;
	}

	//alarm after timeLimit seconds
	signal(SIGALRM, setdoneflag);
	alarm(timeLimit);

	int shmid = shmget(SHMKEY, sizeof(messageStruct), 0666 | IPC_CREAT);
	clock = (messageStruct *)shmat(shmid, NULL, 0);

	clock -> seconds = 0;
	clock -> nanosecs = 0;
	clock -> pid = 0;
	clock -> shmMsg = 0;


	sem = sem_open("clockSem", 1);


	while (opt != -1) {

		opt = (getopt(argc, argv, "s:t:l:h"));
		switch (opt)
		{

			case 's':
				maxProcesses = atoi(optarg);
				fprintf(stderr, "Testing maxProcesses: %d\n", maxProcesses);
				break;
			case 't':
				killTime = atoi(optarg);
				fprintf(stderr, "Testing killTime: %d\n", killTime);
				break;
			case 'l':
				fileName = malloc(strlen(optarg));
				strcpy(fileName, optarg);
				fprintf(stderr, "Testing fileName: %s\n", fileName);
				break;
			case 'h':
				puts("To execute oss, you may specify one of four arguments: -s x, -l filename, -t z, -h\n");
				puts("In ./oss -s x, 'x' specificies the max number of user processes spawned.\n");
				puts("In ./oss -l filename, 'filename' specificies the log file used.\n");
				puts("In ./oss -t z, 'z' specificies the time in seconds when the master will terminate itself and all children.\n");
				puts("Example invocation with one argument: ./oss -s 100\n");
				puts("Example invocation with three argments: ./oss -s 100 -t 2 -l logfile\n");
				break;
			case '?':	
				errno = 8;
				perror("oss: ERROR: An argument must be included after executable. For help, run: ./oss -h\n");
				break;
			default:
				break;
		}

	}

	if (fileName == NULL){
		fileOut = fopen("logfile", "w");
	} else {
		fileOut = fopen(fileName, "w");
	}
	



	while (processCount < maxTotal && !doneflag){
		if (currentProcesses<maxProcesses){
			if ((childPid = fork()) == 0){
				execlp("./worker", "./worker", (char*)NULL);

				fprintf(stderr, "%sFailed exec worker!\n", argv[0]);
				_exit(1);
		}
		if (childPid == -1){
			printf("\n\n\n\n\n\n\n\n\n\n\n\nFORK FAILED AT PROCESS: %d\n\n\n\n\n\n\n\n", processCount); 
		}
		
		printf("Childpid: %d\n", childPid);
		printf("Process number: %d\n", processCount);
		printf("Current Processes: %d\n", currentProcesses);
		processCount += 1;
		currentProcesses += 1;
	}
		// if (currentProcesses >= maxProcesses){
		// 	wait(NULL);
		// 	currentProcesses -= 1;
		// }


		

		sem_wait(sem);
            if(clock->pid > 0){
                printf("Child finished in loop 1: %d Time: %1.2f\n", clock->pid, clock->shmMsg);
		
		clock->pid = 0;
                currentProcesses -= 1;
            }
            clock->nanosecs += 10000;
            if (clock->nanosecs >= 1000000000){
                clock->seconds += 1;
                clock->nanosecs = clock->nanosecs % 1000000000;
            }
            if (clock->seconds >= 2){
                clock->nanosecs = 0;
            }
        //printf("Nanoseconds: %d\n", clock->nanosecs);

        sem_post(sem);

		



	}

	clock->seconds = 2;
    clock->nanosecs = 0;

	while((waitpid = wait(&waitstatus)) > 0){
            sem_wait(sem);
            if(clock->pid > 0){
                printf("Child finished in loop 2: %d Time: %1.2f\n", clock->pid, clock->shmMsg);
                clock->pid = 0;
                currentProcesses -= 1;
            }
            sem_post(sem);
            
    }	
	printf("Processes: %d\n", processCount); 
    printf("End of parent\n");
    fprintf(fileOut, "Test\n");
	shmdt(clock);

	shmctl(shmid, IPC_RMID, NULL);
	
	return 0;

}
