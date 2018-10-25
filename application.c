#include "application.h"


int main(int argc, char** argv){

	int r_e_flag = 0;
	int port, fd;

	if (argc < 3)
	{
		printf("Incorrect number of arguments\n");
		exit(1);
	}

	//port
	if (atoi(argv[1]) == 0)
		port = COM1;
	else if (atoi(argv[1]) == 1)
		port = COM2;
	else
	{
		printf("First argument invalid\n");
		exit(1);
	}

	//
	if (atoi(argv[2]) == 0)
		r_e_flag = EMISSOR_FLAG;
	else if (atoi(argv[2]) == 1)
		r_e_flag = RECEIVER_FLAG;
	else
	{
		printf("Second argument invalid\n");
		exit(1);
	}

	fd = llopen(port, r_e_flag);

	//fflush(NULL);

	char teste[6] = {0x00, 0x04, 0x7e, 0x5d, 0x7d, 0x3e};
	char teste1[255];
	//if(fd > 0)
		llwrite(fd, teste, 6);
		//llread(fd, teste1);
	//llclose(fd, r_e_flag);

	return 0;

}
