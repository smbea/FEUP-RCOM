#include "stateMachine.h"
#include "dataLink.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

struct termios oldtio, newtio;
int send_flag = 1, conta = 1;

int main(int argc, char **argv)
{
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

	fflush(NULL);

	if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
	{
		perror("tcsetattr");
		exit(-1);
	}

	close(fd);

	return 0;
}

int llopen(int port, int r_e_flag)
{
	int fd;
	char *portName;

	if (port == COM1)
		portName = "/dev/ttyS0";
	else if (port == COM2)
		portName = "/dev/ttyS1";
	else
		return -1;

	fd = open(portName, O_RDWR | O_NOCTTY);
	if (fd < 0)
	{
		perror(portName);
		exit(-1);
	}

	if (tcgetattr(fd, &oldtio) == -1)
	{ /* save current port settings */
		perror("tcgetattr");
		exit(-1);
	}

	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;

	/* set input mode (non-canonical, no echo,...) */
	newtio.c_lflag = 0;

	newtio.c_cc[VTIME] = 1; /* inter-character timer unused */
	newtio.c_cc[VMIN] = 0;

	tcflush(fd, TCIOFLUSH);

	if (tcsetattr(fd, TCSANOW, &newtio) == -1)
	{
		perror("tcsetattr");
		exit(-1);
	}

	printf("New termios structure set\n");

	if (r_e_flag == RECEIVER_FLAG)
		open_receiver(fd);
	else if (r_e_flag == EMISSOR_FLAG)
		open_emissor(fd);

	return fd;
}

void open_receiver(int fd)
{
	/*
	st.currentState = START;
	st.currentStateFunc = &stateStart;*/
	initStateMachine(&st,RECEIVER_FLAG,SET);

	unsigned char frame;
	while (st.currentState != END)
	{
		if (read(fd, &frame, 1) > 0)
		{
			(*st.currentStateFunc)(&st, frame);

			printf("received: %X\n", frame);
		}
	}

	send_UA(fd);

	sleep(3);
}

void atende(int signo)
{
	printf("alarme # %d\n", conta);
	send_flag = 1;
	conta++;
}

void open_emissor(int fd)
{

	int res;

	/*st.currentState = START;
	st.currentStateFunc = &stateStart;*/

	initStateMachine(&st,EMISSOR_FLAG,UA);

	struct sigaction act;
	act.sa_handler = atende;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;

	if (sigaction(SIGALRM, &act, NULL) == -1)
	{
		printf("Error\n");
		exit(-1);
	}

	unsigned char teste;

	while (conta < 4)
	{
		if (send_flag)
		{

			printf("writing message\n");
			send_SET(fd);

			alarm(3); // activa alarme de 3s
			printf("sent alarm\n");
			send_flag = 0;
		}

		while (1)
		{
			res = read(fd, &teste, 1);
			if (res > 0)
			{
				printf("%x\n", teste);
				(*st.currentStateFunc)(&st, teste);
			}
			if (st.currentState == END || send_flag)
				break;
		}

		if (st.currentState == END)
			return;
	}
}

void send_SET(int fd)
{
	unsigned char buf[5] = {FLAG, SENT_BY_EMISSOR, SET, SENT_BY_EMISSOR ^ SET, FLAG};
	write(fd, buf, 5);
	printf("sent SET packet\n");
}

void send_UA(int fd)
{
	// TODO
	unsigned char buf[5] = {FLAG, SENT_BY_RECEPTOR, UA, SENT_BY_RECEPTOR ^ UA, FLAG};
	write(fd, buf, 5);
	printf("sent UA packet\n");
}

int send_DISC(int fd, int r_e_flag)
{
	unsigned char buf[5] = {FLAG, 0 , DISC, 0, FLAG};

	if (r_e_flag == SENT_BY_EMISSOR){
			buf[1] = SENT_BY_EMISSOR;
			buf[3] = SENT_BY_EMISSOR ^ DISC;
		}
	else if (r_e_flag == SENT_BY_RECEPTOR){
			buf[1] = SENT_BY_RECEPTOR;
			buf[3] = SENT_BY_RECEPTOR ^ DISC;
		}
	else{
		return -1;
	}
	write(fd, buf, 5);

	printf("sent DISC packet\n");

	return 0;
}

//How to handle an error in a last UA sent. After sending the last UA the receiver closes/disconnects, if this UA is not received by the receiver
//should the receiver time-out and also disconnect/close or should it stay in a infinite wait for the last UA? Use timeout
void close_receiver(int fd)
{

	int res;
	//Waits for first DISC flag

	initStateMachine(&st,RECEIVER_FLAG,DISC);

	unsigned char frame;
	while (st.currentState != END)
	{
		if (read(fd, &frame, 1) > 0)
		{
			(*st.currentStateFunc)(&st, frame);

			printf("received: %X\n", frame);
		}

	} //DISC flag received

	//Waits for UA flag to end the data link, while it does not receive UA flag tries to resend DISC flag
	initStateMachine(&st,RECEIVER_FLAG,UA);

	struct sigaction act;
	act.sa_handler = atende;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;

	if (sigaction(SIGALRM, &act, NULL) == -1)
	{
		printf("Error\n");
		exit(-1);
	}

	unsigned char teste;
	
	printf("writing message\n");
	send_DISC(fd, RECEIVER_FLAG); //sends DISC flag back to the emissor

	//if doesn't receive UA back in 3 seconds ends anyway

	alarm(3); // activa alarme de 3s
	printf("sent alarm\n");
	send_flag = 0;


	while (1)
	{
		res = read(fd, &teste, 1);
		if (res > 0)
		{
			printf("%x\n", teste);
			(*st.currentStateFunc)(&st, teste);
		}
		if (st.currentState == END || send_flag)
			break;
	}

	if(send_flag) printf("Wanrning: UA was not received");
}

void close_emissor(int fd)
{

	int res;

	initStateMachine(&st,EMISSOR_FLAG,DISC);

	struct sigaction act;
	act.sa_handler = atende;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;

	if (sigaction(SIGALRM, &act, NULL) == -1)
	{
		printf("Error\n");
		exit(-1);
	}

	unsigned char teste;

	while (conta < 4)
	{
		if (send_flag)
		{

			printf("writing message\n");
			send_DISC(fd, SENT_BY_EMISSOR);

			alarm(3); // activa alarme de 3s
			printf("sent alarm\n");
			send_flag = 0;
		}

		while (1)
		{
			res = read(fd, &teste, 1);
			if (res > 0)
			{
				printf("%x\n", teste);
				(*st.currentStateFunc)(&st, teste);
			}
			if (st.currentState == END || send_flag)
				break;
		}

		if (st.currentState == END)
		{
			send_UA(fd);
			return;
		}
	}
}

int llwrite(int fd, char * buffer, int length) {

}

void byteStuffing(char * buffer, int length, char * stuffedBuffer, int stuffedLength) {
	/**
	 * Compute the BCC
	 */
	unsigned int i, j;
	unsigned char bcc = 0;
	// compute BCC
	for(i = 0; i < length; i++) {
		bcc ^= buffer[i];
	}

	/**
	 * Parse the data and perform byte stuffing
	 */
	unsigned char escapeChar = 0x7d;
	unsigned char flagChar = 0x7e;

	for(i = 0; i < length; i++, j++) {
		if(buffer[i] == flagChar) {
			stuffedBuffer[j] = escapeChar;
			stuffedBuffer[++j] = flagChar ^ 0x20;  
		} else if(buffer[i] == escapeChar) {
			stuffedBuffer[j] = escapeChar;
			stuffedBuffer[++j] = escapeChar ^ 0x20;
		} else if(buffer[i] == bcc) {
			stuffedBuffer[j] = escapeChar;
			stuffedBuffer[++j] = bcc ^ 0x20; // not sure about this
		} else {
			stuffedBuffer[j] = buffer[i];
		}
	}
}

int llclose(int fd, int r_e_flag)
{

	if (r_e_flag == RECEIVER_FLAG)
		close_receiver(fd);
	else if (r_e_flag == EMISSOR_FLAG)
		close_emissor(fd);

	fflush(NULL);

	if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
	{
		perror("tcsetattr");
		exit(-1);
	}

	close(fd);

	return 1; //positive return value for success
}
