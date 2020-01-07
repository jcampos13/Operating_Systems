#include "registration.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <semaphore.h>
#include <fcntl.h>
#include <memory.h>

using namespace std;
CLASS *class_ptr;

void *memptr;
char *pname;
int shmid, ret;
void rpterror(char *), srand(), perror(), sleep();
void sell_seats();
sem_t * sem;

int main(int argc, char* argv[])
{

	//sem_open creates new semaphores or opens existing ones. In this case, the children processes will need to open the semaphore created by the parent process. 
	//The processes opens the semaphore using the name it was given (parameter 1). The second argument is ignored since the parent creates a new semaphore "/semaphore 1" before spawning the child processes each time. 
	//The parent process calls sem_unlink() after each process associated with the semaphore closes it. This ensures that the semaphore doesn't already exists even after the program ends.
	sem=sem_open("/semaphore1", 0);

	//SEM_FAILED is the value returned when the system call sem_open() fails. The code inside the if statement alerts the user when this happens.
	if(sem==SEM_FAILED)
	{
		cout<<"SEMAPHORE OPENING FAILED!"<<errno;
	}
	
	if (argc < 2) 
	{
		fprintf (stderr, "Usage:, %s shmid\n", argv[0]);
		exit(1);
	}
	
	//stores the name of the process
	pname = argv[0];

	//stores the id provided by argv[1], in the format expressed by "%d" (indicating integer), to the parameter "shmid".
	sscanf (argv[1], "%d", &shmid);

	//explained in shmp1.cpp
	memptr = shmat (shmid, (void *)0, 0);

	//explained in shmp1.cpp
	if (memptr == (char *)-1 ) 
	{
		rpterror ((char*)"shmat failed");
		exit(2);
	}

	class_ptr = (struct CLASS *)memptr;
	sell_seats();
	
	//explained in shmp1.cpp
	ret = shmdt(memptr);
	exit(0);
}

void sell_seats() 
{
	int all_out = 0;

	/*srand() seeds the pseudo random number generator used by the rand() function used in 
	the while loop below. The seed in this case is the pid of the process
	*/
	srand ( (unsigned) getpid() );

	while ( !all_out) 
	{   	
		/* loop to sell all seats */


		//sem_wait() locks (decrements) the semaphore that is pointed to by "sem". Since the semaphore is initialized to 1, the decrement results in 0. 
		//This blocks other processes until it becomes possible to perform the decrement. In other words, until the value is above 0 again.
		sem_wait(sem);

		/* puts the process to sleep for an amount of time, then decreases the amount of seats available. Before printing out the new count of seats, the process sleeps again.
		Finally, it prints the seat count until there are no more seats left.*/
		if (class_ptr->seats_left > 0) 
		{
			sleep ( (unsigned)rand()%5 + 1);
			class_ptr->seats_left--;
			sleep ( (unsigned)rand()%5 + 1);
			cout << pname << " SOLD SEAT -- "  << class_ptr->seats_left << " left" <<endl;
			
		}
		
		else
		{
			all_out++;
			cout << pname << " sees no seats left" << endl;
		}

		//sem_post() increments (unlocks) the semaphore that is pointed to by "sem". 
		//If the increment results in a value greater than 0, another process that was blocked by sem_wait() will be able to proceed and lock the semaphore.
		sem_post(sem);

		sleep ( (unsigned)rand()%10 + 1);
	}
}

void rpterror(char* string)
{
	char errline[50];
	sprintf (errline, "%s %s", string, pname);
	perror (errline);
}
