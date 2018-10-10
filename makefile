
all: oss worker

oss: 
	gcc -Wall master.c -o oss -pthread

worker:
	gcc -Wall worker.c -o worker -pthread

clean:
	rm worker
	rm oss
