
all: oss worker

oss: 
	gcc -Wall master.c -o oss

worker:
	gcc -Wall worker.c -o worker 

clean:
	rm worker
	rm oss
