/*
Header file to be used with
shmp1.c and shmc1.c. Represents the shared memory segment that the processes will use. The seats_left variable is what the processes will decrement.
 */
struct CLASS {
char class_number[6];
char date[7];
char title[50];
int seats_left;
};
