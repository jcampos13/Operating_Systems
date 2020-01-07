/*The structure that is going to be the shared memory segment*/
struct CLASS 
{
	char class_number[6];
	char date[7];
	char title[50];
	int seats_left;
};
