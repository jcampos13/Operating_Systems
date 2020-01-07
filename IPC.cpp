// use_msgQ.cpp
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
using namespace std;

// declare my global message buffer
struct buf 
{
    long mtype; // required
    char greeting[50]; // message content
};

void child_proc_one(int qid) 
{
    buf msg;
    int size = sizeof(msg)-sizeof(long);

    //The msgrcv() system call is used to receive messages from the queue
    //In order to use the msgrcv() system call, the process must have permission to receive messages
    //The qid paramater is the identifier for the message queue that is to be read from
    //The "msg" parameter is a data structure for the message data
    //The "size" parameter is the size of the greeting member of the buf data structure in bytes
    //The integer 113 is a parameter that specifies what kind of message to receive
    //A value greater than 0 indicates that the first message of type msgtyp is to be received
    //The integer 0 is a parameter that determines that action to be taken when the message of the type requested isn't in the queue
    //A value equal to 0 indicates to suspend execution 
    msgrcv(qid, (struct msgbuf *)&msg, size, 113, 0); 

    //getpid() is used to retrieve the process ID that was given to this process
    cout << getpid() << ": gets message" << endl;
    cout << "message: " << msg.greeting << endl;

    //appends additional message
    strcat(msg.greeting, " and Adios.");

    //process reply
    cout << getpid() << ": sends reply" << endl;
    msg.mtype = 114; 

    //The msgsnd() system call is used to write messages to the queue
    //if there is enough space in the queue, the call succeeds.
    //if there is NOT enough space then the default is to block until space becomes available
    // prepare message with type mtype = 114
    //The qid paramater is the identifier for the message queue that is to be written to
    //The "msg" parameter ia a copy of the message that is going to be sent to the queue
    //"size" parameter is the size of the message in bytes
    //The integer 0 indicates to ignore it
    msgsnd(qid, (struct msgbuf *)&msg, size, 0);
    cout << getpid() << ": now exits" << endl;
}

void child_proc_two(int qid) 
{
    buf msg;
    int size = sizeof(msg)-sizeof(long);

    // prepare a message that will never be read
    msg.mtype = 12;
    strcpy(msg.greeting, "Fake message");

    //writes message of type 12 to the queue 
    msgsnd(qid, (struct msgbuf *)&msg, size, 0);

    // prepare my message to send
    strcpy(msg.greeting, "Hello there");
    cout << getpid() << ": sends greeting" << endl;
    msg.mtype = 113; 

    // set message type mtype = 113
    msgsnd(qid, (struct msgbuf *)&msg, size, 0);

    //Requests to receive message of type 114 from the queue with process ID "qid"
    //The integer 0 parameter indicates what to do if a message of this type is not in the queue
    //0 instructs to suspend execution
    msgrcv(qid, (struct msgbuf *)&msg, size, 114, 0); 

    
    cout << getpid() << ": gets reply" << endl;
    cout << "reply: " << msg.greeting << endl;
    cout << getpid() << ": now exits" << endl;
}

int main() 
{
    //"msgget()" is a system call which returns the value of the message queue identifier (a non-negative integer), if it succeeds
    //if the call fails, a -1 is returned
    //IPC_PRIVATE is a system-generated unique key. Using this parameter ensures that no other process will have this key
    //IPC_CREAT is a predefined constant in the "sys/ipc.h" header file used to create a queue if it doesn't already exist
    //IPC_EXCL, when used along with IPC_CREAT, returns a -1 to indicate failure. IPC_EXCL alone does not do anything
    //the value "0600" specifies the valid operations. The value is obtained by adding the octal values for the operation permissions
    //In this case, the octal value represents the addition of the "Read by user" (00400) and "Write by user" (00200) permissions 
    int qid = msgget(IPC_PRIVATE, IPC_EXCL|IPC_CREAT|0600);

    //spawning first child process
    //the pid_t type can represent a process ID
    pid_t cpid = fork();

    //the fork method returns a value of 0 if the child process has been created successfully
    //the PID of the child process is returned in the parent using fork()
    //Otherwise, if the creation of the child process fails, the value returned is a -1
    if (cpid == 0) 
    { 
        child_proc_one(qid);
        exit(0);
    }

    //spawning the second child process
    cpid = fork();

    if (cpid == 0)
    {
        child_proc_two(qid);
        exit(0);
    }

    //msgctl() allows operations on the message queue to be done
    //The "qid" parameter indicates which message queue is going to be used
    //The IPC_RMID parameter requests the queue to be removed and destroy all messages on the message queue
    //The NULL parameter is where a pointer to the data structure associated to the queue would be
    //The parameter is NULL because IPC_RMID is a command that can only be ran by a thread with privilages or one with the same user ID as the owner or creator
    //Therefore passing a pointer to the msqid_ds is not needed
    while(wait(NULL) != -1); // waiting for both children to terminate
    msgctl(qid, IPC_RMID, NULL); 

    //getpid() returns the process ID of the parent
    //since the child proccesses are terminated and were made with the fork() function
    //the process ID of the parent is printed
    cout << "parent proc: " << getpid() << " now exits" << endl;
    exit(0);
}
