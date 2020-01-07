#include "registration.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <memory.h>

using namespace std;
CLASS myclass = { "1001", "120119", "Operating Systems", 15 };
#define NCHILD 3

int shm_init( void * );
void wait_and_wrap_up( int [], void *, int );
void rpterror( char *, char * );

int main(int argc, char *argv[])
{
	int child[NCHILD], i, shmid;
	void *shm_ptr;
	char ascshmid[10], pname[14];

	//shmid holds the identifier of the shared memory segment once it has been returned by shm_init()
	shmid = shm_init(shm_ptr);
	//copies the identifier into a character array to send it to the child processes
	sprintf (ascshmid, "%d", shmid);

		for (i = 0; i < NCHILD; i++) 
		{
			//three child process are spawned in the image of the parent
			child[i] = fork();
	
			//establish whether all children were created successfully
			switch (child[i]) 
			{
				//child process creation failed...
				case -1:
					rpterror ((char *)"fork failure", pname);
					exit(1);
				//given that the fork() was successful (the children were spawned successfully)...
				case 0:
					//sends a formatted ouput to a string pointed to by pname
					sprintf (pname, "shmc%d", i+1);
					/*load a new program into the childs address space. The program loaded is the program for shmc1. The name of the new process (pname) along with the identifier of the shared memory segment are passed as parameters 2, and 3 respectively. The last value is NULL*/ 
					execl("shmc1", pname, ascshmid, (char *)0);
					perror ("execl failed");
					exit (2);
			}
		}

	wait_and_wrap_up (child, shm_ptr, shmid);
}


int shm_init(void *shm_ptr)
{
	int shmid;
	/* shmget() returns the identifier of the shared memory segment associated with the first argument. the first argument is a call to system call "ftok()", which converts a pathname and 	
	project identifier to a system V IPC key. In other words, the system call generates an IPC key that can be used with other system calls in IPC such as msgget(), semget(), or shmget(). 
	the return value of ftok() is the same for all pathnames that name the same file, and when the value for its second argument is the same, in which only the least significant 8 bits are used. The return value of ftok() is a key of type key_t. 
	shmget() can either be used to obtain the identifier of a previously created shared memory or to create a new set.
	A new shared memory segment with a size equal to the value of the second argument in shmget() is made if the value IPC_PRIVATE is passed as a first parameter or the first parameter isn't IPC_PRIVATE, no shared memory segment corresponding to the first parameter exists, and IPC_CREAT is specified as a third argument in shmget().   
	*/
	shmid = shmget(ftok(".",'u'), sizeof(CLASS), 0600 | IPC_CREAT);

	/*given the the creation of the shared memory segment fails, a -1 is returned and errno is set with a value to indicate the reason for why the call failed.
	*/
	if (shmid == -1) 
	{
		perror ("shmget failed");
		exit(3);
	}

	/*shmat system call attaches the shared memory segment identified by the parameter "shmid" to the address space of the calling process.
	the second parameter,"shmaddr", specifies the address of the memory segment that is going to be attached. If this parameter is set to 0, the kernel will attempt to find an 
	unmapped segment to use. The final parameter simply serves as a set of flags that indicate the specific shared memory conditions and options to implement. In this case the parameter denoted is 0. 		
	Therefore, the segment is attached for read and write and the process must have read and write permission for the segment. The other option for this parameter is to use SHM_RDONLY, which means the 		
	segment is going to be attached for reading and the process must have read permission for the segment.
	*/
	shm_ptr = shmat(shmid, (void * ) 0, 0);

	/*in the case that shmat fails, a -1 is returned to 
	notify that the shared memory segment was not attached. Also, a value is set 
	for errno, which provides further information on why the call failed.
	*/
	if (shm_ptr == (void *) -1) 
	{
		perror ("shmat failed");
		exit(4);
	}
	
	/*The memcpy() function is used to copy a block of memory from one location to another. In this case, a pointer to the struct of this process is copied to the pointer to the start address of the shared memory segment. In other words, the shared memory segment pointer (shm_ptr) points to the struct "myclass"
	*/
	memcpy (shm_ptr, (void *) &myclass, sizeof(CLASS) );

	//the identifier of the shared memory segment is returned
	return (shmid);
}

void wait_and_wrap_up(int child[], void *shm_ptr, int shmid)
{
	int wait_rtn, w, ch_active = NCHILD;
	//loops while the count of active processes is not 0...
	while (ch_active > 0) 
	{
		//the wait() system call simply serves so that the parent process waits for all its children to terminate before it can move on.
		wait_rtn = wait( (int *)0 );
		for (w = 0; w < NCHILD; w++)
		{
			if (child[w] == wait_rtn) 
			{
				//when a child ends, the counter that holds the value of active processes is decremented by 1.
				ch_active--;
				break;
			}
		}
	}
	cout << "Parent removing shm" << endl;
	/*The shmdt() system call detaches the shared memory segment located at the address indicated by the parameter "shm_ptr" from the address space of the process calling it. 
	*/
	shmdt (shm_ptr);
	/*The shmctl() system call performs control operations on the memory segment. More specifically, the first parameter is the identifier of the
	memory segment and the second argument indicates the operation that is to be performed on the segment. The IPC_RMID value here says to mark the
	segment to be destroyed after the last process detaches from it. The value that tracks this is shm_nattch. When this value is 0 the segment is deleted. The last parameter is a pointer to the shmid_ds
	structure that contains the number of proccesses attached to the segment. In this case the value is not needed.
	*/
	shmctl (shmid, IPC_RMID, (struct shmid_ds *) 0);
	exit (0);
}

void rpterror(char *string, char *pname)
{
	char errline[50];
	sprintf (errline, "%s %s", string, pname);
	perror (errline);
}
