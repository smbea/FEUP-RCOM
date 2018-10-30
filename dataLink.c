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
unsigned char ns = S0;
unsigned char nr = 0x40;


void genNextNs(){
	if(ns == S1){
		ns = S0;
	}
	else{
		ns = S1;
	}
}

void genNextNr(unsigned char received_ns){
	if(received_ns == S0){
		nr = RR1;
	}
	else{
		ns = RR0;
	}
}


int llopen(int port, int status)
{
	int fd;
	char *portName;

	dataLink.baudRate = B38400;
	dataLink.timeout = 3;
	dataLink.numTransmissions=3;

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
	newtio.c_cflag = dataLink.baudRate | CS8 | CLOCAL | CREAD;
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

	if (status == RECEIVER_FLAG)
		open_receiver(fd);
	else if (status == EMISSOR_FLAG)
		open_emissor(fd);

	return fd;
}

void open_receiver(int fd)
{
	/*
	st.currentState = START;
	st.currentStateFunc = &stateStart;*/
	initStateMachine(&st, SENT_BY_EMISSOR, SET);

	unsigned char frame;
	while (st.currentState != END)
	{
		if (read(fd, &frame, 1) > 0)
		{
			(*st.currentStateFunc)(&st, frame);

			//printf("received: %X\n", frame);
		}
	}

	send_UA(fd);

	//sleep(3);
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

	initStateMachine(&st, SENT_BY_RECEPTOR, UA);

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

	while (conta <= dataLink.numTransmissions)

	{
		if (send_flag)
		{

			printf("writing message\n");
			send_SET(fd);

			alarm(dataLink.timeout); // activa alarme de 3s
			printf("sent alarm\n");
			send_flag = 0;
		}

		while (1)
		{
			res = read(fd, &teste, 1);
			if (res > 0)
			{
				//printf("%x\n", teste);
				(*st.currentStateFunc)(&st, teste);
			}
			if (st.currentState == END || send_flag)
				break;
		}

		if (st.currentState == END){
			sigignore(SIGALRM);
			return;
		}
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
	unsigned char buf[5] = {FLAG, SENT_BY_RECEPTOR, UA, SENT_BY_RECEPTOR ^ UA, FLAG};
	write(fd, buf, 5);
	printf("\n %x %x %x %x %x\n", buf[0], buf[1], buf[2], buf[3], buf[4]);
	printf("sent UA packet\n");
}

int send_I(int fd, unsigned char *data, int length, byte bcc2)
{

	int res = 0, i = 0;
	unsigned char buf[260] = {FLAG, SENT_BY_EMISSOR, ns, SENT_BY_EMISSOR ^ ns};
	int j = 4;

	for (i = 0; i < length; i++)
	{
		//printf("%c\n", data[i]);
		buf[j] = data[i];
		j++;
	}

	buf[j++] = bcc2;
	buf[j] = FLAG;
	for(i = 0; i <= j; i++) {
		printf("%d:%x\n", i, buf[i]);
	}
	res = write(fd, buf, j+1);
	//char buf2[6] = {0x00, 0x04, 0x7d, 0x5d, 0x7d, 0x5e};
	//res = write(fd, buf2, 6);
	if (res > 0)
	{
		printf("sent I packet\n");
		return res;
	}
	return -1;
}

int send_DISC(int fd, int status)
{
	unsigned char buf[5] = {FLAG, 0, DISC, 0, FLAG};

	if (status == EMISSOR_FLAG)
	{
		buf[1] = SENT_BY_EMISSOR;
		buf[3] = SENT_BY_EMISSOR ^ DISC;
	}
	else if (status == RECEIVER_FLAG)
	{
		buf[1] = SENT_BY_RECEPTOR;
		buf[3] = SENT_BY_RECEPTOR ^ DISC;
	}
	else
	{
		return -1;
	}

	printf("DISC: %x\n", buf[2]);
	write(fd, buf, 5);

	printf("sent DISC packet\n");

	return 0;
}

//How to handle an error in a last UA sent. After sending the last UA the receiver closes/disconnects, if this UA is not received by the receiver
//should the receiver time-out and also disconnect/close or should it stay in a infinite wait for the last UA? Use timeout
void close_receiver(int fd, int status)
{

	int res;
	//Waits for first DISC flag

	initStateMachine(&st, SENT_BY_EMISSOR, DISC);

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
	initStateMachine(&st, SENT_BY_RECEPTOR, UA);

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

	while (conta <= dataLink.numTransmissions)
	{ //4 tentativas de alarme
		if (send_flag)
		{

			printf("writing message\n");
			send_DISC(fd, status); //sends DISC flag back to the emissor

			alarm(dataLink.timeout); // activa alarme de 3s
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

		if (st.currentState == END){
			sigignore(SIGALRM);
			return;
		}
	}
	printf("Communication closed by timeout. Last UA not received.\n");
}

void close_emissor(int fd, int status)
{
	int res;

	initStateMachine(&st, SENT_BY_RECEPTOR, DISC);

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

	while (conta <= dataLink.numTransmissions)
	{
		if (send_flag)
		{

			printf("writing message\n");
			send_DISC(fd, status);

			alarm(dataLink.timeout); // activa alarme de 3s
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

		if (st.currentState == END){
			send_UA(fd);
			sigignore(SIGALRM);
			return;
		}
	}
}

unsigned char getBCC(unsigned char* buffer, int length, int status)
{
		int i;
		unsigned char bcc = 0;

		for(i = 0; i<(length-1);i++){
			bcc = bcc ^ buffer[i];
		}

		return bcc;
}

int llwrite(int fd, unsigned char *buffer, int length)
{

	unsigned char stuffedBuffer[260];
	int res2 = 0, res1 = 0, newLength = length;
	unsigned char bcc2;
	unsigned char singleByte = 0;
	int i = 0;
	conta = 1, send_flag = 1;

	if (ns == 0x40)
		initStateMachine(&st, SENT_BY_EMISSOR, RR0);
	else
		initStateMachine(&st, SENT_BY_EMISSOR, RR1);

	struct sigaction act;
	act.sa_handler = atende;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;

	if (sigaction(SIGALRM, &act, NULL) == -1)
	{
		printf("Error\n");
		exit(-1);
	}

	bcc2 = getBCC(buffer, length, EMISSOR_FLAG);
	byteStuffing(buffer, length, stuffedBuffer, &newLength);

	while (conta <= dataLink.numTransmissions)
	{
		if (send_flag)
		{
			printf("writing frame\n");
			res1 = send_I(fd, stuffedBuffer, newLength, bcc2);
			if (res1 < 0)
				return -1;

			alarm(dataLink.timeout); // activa alarme de 3s
			send_flag = 0;
		}

		while (1)
		{
			res2 = read(fd, &singleByte, 1);
			if (res2 > 0)
			{
				//printf("%x\n", teste);
				(*st.currentStateFunc)(&st, singleByte);
				i++;
			}

			if (st.currentState == END || send_flag)
				break;
		}

		if (st.currentState == END)
		{	genNextNs();
			return res1;
		}
	}
	return res1;
}


void byteStuffing(unsigned char *buffer, int length, unsigned char *stuffedBuffer, int* newLength)
{
	/**
	 * Compute the BCC
	 */
	unsigned int i;
	unsigned int j = 0;

	/**
	 * Parse the data and perform byte stuffing
	 */
	unsigned char escapeChar = 0x7d;
	unsigned char flagChar = 0x7e;

	for (i = 0; i < length; i++, j++)
	{
		if (buffer[i] == flagChar)
		{
			stuffedBuffer[j] = escapeChar;
			stuffedBuffer[++j] = flagChar ^ 0x20;
		}
		else if (buffer[i] == escapeChar)
		{
			stuffedBuffer[j] = escapeChar;
			stuffedBuffer[++j] = escapeChar ^ 0x20;
		}
		else
		{
			stuffedBuffer[j] = buffer[i];
		}
	}
	(*newLength) = j;
}

int byteDestuffing(unsigned char* stuffedBuffer, int length, unsigned char* destuffedBuffer)
{
	int indexS = 0, indexD = 0;
	unsigned char escapeChar = 0x7d;
	unsigned char flagChar = 0x7e;

	for(indexS = 0; indexS < length; indexS++)
	{

		//printf("found flag %x\n",flagChar ^ 0x20);
		if(stuffedBuffer[indexS] != escapeChar)
		{
			destuffedBuffer[indexD] = stuffedBuffer[indexS];
		}
		else{
			if(stuffedBuffer[indexS+1] == (escapeChar ^ 0x20)){
				destuffedBuffer[indexD] = escapeChar;
				indexS++;
			}else if(stuffedBuffer[indexS+1] == (flagChar ^0x20)){
				destuffedBuffer[indexD] = flagChar;
				indexS++;
			}
		}
		indexD++;
	}
	return indexD;
}

int send_R(int fd, int success, unsigned char received_ns)
{
	unsigned char buf[5] = {FLAG, SENT_BY_EMISSOR, 0, 0, FLAG};
	if(success)
	{
		genNextNr(received_ns);

		buf[2] = nr;
		buf[3] = nr ^ SENT_BY_EMISSOR;
	}
	else{
		if(ns == S0)
		{
			buf[2] = REJ1;
			buf[3] = REJ1 ^ SENT_BY_EMISSOR;
		}
		else if(ns == S1)
		{
			buf[2] = REJ0;
			buf[3] = REJ0 ^ SENT_BY_EMISSOR;
		}
		else
			return -1;
	}

	printf("RR/REJ: %x\n", buf[2]);
	write(fd, buf, 5);

	//testing
	int i;
	for(i = 0; i<5;i++){
		printf("%x ", buf[i]);
	}

	printf("\n Sent response packet\n");

return 0;
}

unsigned char * extractData(unsigned char * buffer, unsigned char * data, int length)
{
	int index = 0;
	int i;
	for(i = 0; i < (length-1); i++)
		data[index++] = buffer[i];

	return data;
}

int llread(int fd, unsigned char *buffer)
{
	int res = 0, res2 = 0, destuffedSize;
	int bccSuccess = 0;
	unsigned char destuffed[256];
	unsigned char buf = 0;
	int i = 0;

	initStateMachine(&st, SENT_BY_EMISSOR, ns);

	struct sigaction act;
	act.sa_handler = atende;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;

	if (sigaction(SIGALRM, &act, NULL) == -1)
	{
		printf("Error\n");
		exit(-1);
	}
	while (1)
	{
		res = read(fd, &buf, 1);
		if (res > 0)
		{
			printf("%x\n", buf);
			(*st.currentStateFunc)(&st, buf);
		}
		if (st.currentState == END)
			break;

		if (st.currentState == DATA){
			dataLink.frame[i] = buf;
			i++;
		}
				
	}
	
	destuffedSize = byteDestuffing(dataLink.frame, i, destuffed);

	int j;

	for(j = 0; j < destuffedSize; j++)
	{
		printf("DB: %x\n", destuffed[j]);
	}

	unsigned char x = getBCC(destuffed, destuffedSize , RECEIVER_FLAG);
	printf("BCC: %x\n", x);
	printf("BCC recieved: %x\n", destuffed[destuffedSize - 2]);
	if(getBCC(destuffed, destuffedSize , RECEIVER_FLAG) == destuffed[destuffedSize - 2])
	{
		printf("BCC is correct \n");
		bccSuccess = 1; //BCC calculated from data is equal to BCC2 received
	}

	extractData(destuffed,buffer,destuffedSize);
	
	res2 = send_R(fd, bccSuccess,buffer[2]);

	return res2;
}


int llclose(int fd, int status)
{
	printf("\nLLCLOSE\n");
	conta = 1, send_flag = 1;

	if (status == RECEIVER_FLAG)
		close_receiver(fd, status);
	else if (status == EMISSOR_FLAG)
		close_emissor(fd, status);

	fflush(NULL);

	if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
	{
		perror("tcsetattr");
		return -1; //negative return for error
	}

	close(fd);

	return 1; //positive return value for success
}
