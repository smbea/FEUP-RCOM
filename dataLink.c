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

#define frameSize 522

struct termios oldtio, newtio;
int send_flag = 1, alarmRaisesCnt = 1;
unsigned char ns = S0;
unsigned char nr = RR1;

/**
 * @brief Handler to be called upon alarm signals
 * 
 * @param signo The signal identifier
 */
void alarmHandler(int signo) {
	printf("alarme # %d\n", alarmRaisesCnt);
	send_flag = 1;
	alarmRaisesCnt++;
}

/**
 * @brief Subscribes to alarm signals
 * 
 * @retval 0 Successfully installed signals handler
 * @retval -1 Failed to install the handler, with errno set
 */
int alarmSubscribeSignals(void * handler) {
	struct sigaction act;
	act.sa_handler = handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;

	return sigaction(SIGALRM, &act, NULL);
}

/**
 * @brief Processes the expected SET packet sent by the transmitter machine
 * One the SET packet is received and acknowledged, it sends an UA packet
 * 
 * @param fd File descriptor for the serial port interface used by this host machine
 */
static void open_receiver(int fd) {
	initStateMachine(&st, SENT_BY_EMISSOR, SET);

	unsigned char frame;
	while (st.currentState != END) {
		if (read(fd, &frame, 1) > 0)
			(*st.currentStateFunc)(&st, frame);
	}

	send_UA(fd);
}

/**
 * @brief Sends a SET packet through the serial port and waits for an acknowledgement UA packet
 * 
 * @param fd 
 * @retval 0 Connection established successfully
 * @retval -1 Timeout. Couldn't get any response from receiver machine
 */
static int open_emissor(int fd) {
	int res;

	// set state machine to expect a UA packet
	initStateMachine(&st, SENT_BY_RECEPTOR, UA);

	// install handler for alarm signals
	if(alarmSubscribeSignals(alarmHandler)) {
		perror("open_emissor:");
		return -1;
	}

	// attempt to send the SET packet and wait for UA response
	while (alarmRaisesCnt <= dataLink.numTransmissions) {

		if (send_flag) {
			send_SET(fd);
			alarm(dataLink.timeout);
			send_flag = 0;
		}

		unsigned char input;
		// while the state machine doesn't reach END state
		// or the alarm hasn't raised
		while (!(st.currentState == END) && !send_flag) {
			// read single byte
			res = read(fd, &input, 1);
			if (res == 1) // update state machine if read operation was successful
				(*st.currentStateFunc)(&st, input);
		}

		if (st.currentState == END){
			alarmSubscribeSignals(SIG_IGN);
			return 0;
		}
	}

	// if the max. number of transmissions was reached...
	if(alarmRaisesCnt >= dataLink.numTransmissions)
		return -1;
	else 
		return 0;
}

int llopen(int port, int status) {
	// set default values
	dataLink.baudRate = B38400;
	dataLink.timeout = 3;
	dataLink.numTransmissions=3;
	
	int fd; // file descriptor for terminal
	char *portName; // path to serial port interface

	// validate port
	if (port == COM1)
		portName = "/dev/ttyS0";
	else if (port == COM2)
		portName = "/dev/ttyS1";
	else
		return -1;

	// attempt to open the serial port interface
	if((fd = open(portName, O_RDWR | O_NOCTTY)) < 0) {
		perror(portName);
		return -2;
	}

	// save current terminal interface configuration
	if (tcgetattr(fd, &oldtio) == -1) {
		perror("tcgetattr");
		return -3;
	}

	// setup terminal interface
	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag = dataLink.baudRate | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0; // set input mode (non-canonical, no echo,...)
	newtio.c_cc[VTIME] = 1; // inter-character timer unused
	newtio.c_cc[VMIN] = 0;
	tcflush(fd, TCIOFLUSH); // flushes  both  data  received but not read, and data written but not transmitted

	if (tcsetattr(fd, TCSANOW, &newtio) == -1) { // try to apply new configuration
		perror("tcsetattr");
		return -4;
	}

	// TODO remove this
	printf("New termios structure set\n");

	// depending on if the host machine is the transmitter or receiver
	// send/wait for initial packets to establish connection
	if (status == RECEIVER_FLAG)
		open_receiver(fd);
	else if (status == EMISSOR_FLAG) {
		int ret = open_emissor(fd);
		if(ret) return -5; // transmittor gave out, coudln't establish connection
	}

	// return the file descriptor for the serial port interface
	return fd;
}


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
		nr = RR0;
	}
}


void send_SET(int fd)
{
	unsigned char buf[5] = {FLAG, SENT_BY_EMISSOR, SET, SENT_BY_EMISSOR ^ SET, FLAG};
	write(fd, buf, 5);
	printf("Sent SET frame\n");
}

void send_UA(int fd)
{
	unsigned char buf[5] = {FLAG, SENT_BY_RECEPTOR, UA, SENT_BY_RECEPTOR ^ UA, FLAG};
	write(fd, buf, 5);
	printf("sent UA frame\n");
}

int send_I(int fd, unsigned char *data, int length, byte bcc2)
{

	int res = 0, i = 0;
	unsigned char buf[frameSize] = {FLAG, SENT_BY_EMISSOR, ns, SENT_BY_EMISSOR ^ ns};
	int j = 4;

	for (i = 0; i < length; i++)
	{
		buf[j] = data[i];
		j++;
	}

	buf[j] = FLAG;
	
	res = write(fd, buf, j+1);


	if (res > 0) return res;
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
int close_receiver(int fd, int status)
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

	// install handler for alarm signals
	if(alarmSubscribeSignals(alarmHandler)) {
		perror("open_emissor:");
		return -1;
	}

	unsigned char input;

	while (alarmRaisesCnt <= dataLink.numTransmissions)
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
			res = read(fd, &input, 1);
			if (res > 0)
			{
				printf("%x\n", input);
				(*st.currentStateFunc)(&st, input);
			}
			if (st.currentState == END || send_flag)
				break;
		}

		if (st.currentState == END){
			alarmSubscribeSignals(SIG_IGN);
			return 0;
		}
	}
	printf("Communication closed by timeout. Last UA not received.\n");
	return 0;
}

int close_emissor(int fd, int status)
{
	int res;

	initStateMachine(&st, SENT_BY_RECEPTOR, DISC);

	// install handler for alarm signals
	if(alarmSubscribeSignals(alarmHandler)) {
		perror("open_emissor:");
		return -1;
	}

	unsigned char input;

	while (alarmRaisesCnt <= dataLink.numTransmissions)
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
			res = read(fd, &input, 1);
			if (res > 0)
			{
				printf("%x\n", input);
				(*st.currentStateFunc)(&st, input);
			}
			if (st.currentState == END || send_flag)
				break;
		}

		if (st.currentState == END){
			send_UA(fd);
			alarmSubscribeSignals(SIG_IGN);
			return 0;
		}
	}
	return 0;
}

unsigned char getBCC(unsigned char* buffer, int length)
{
		int i;
		unsigned char bcc = buffer[0];

		for(i = 1; i<length;i++){
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
	alarmRaisesCnt = 1, send_flag = 1;

	if (ns == 0x40)
		initStateMachine(&st, SENT_BY_EMISSOR, RR0);
	else
		initStateMachine(&st, SENT_BY_EMISSOR, RR1);

	// install handler for alarm signals
	if(alarmSubscribeSignals(alarmHandler)) {
		perror("open_emissor:");
		return -1;
	}

	bcc2 = getBCC(buffer, length-2);
	buffer[length-1] = bcc2;
	byteStuffing(buffer, length, stuffedBuffer, &newLength);

	while (alarmRaisesCnt <= dataLink.numTransmissions)
	{
		if (send_flag)
		{
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
				//printf("%x\n", input);
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
		printf("Sent RR: %x\n", buf[2]);
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
		printf("Sent REJ: %x\n", buf[2]);
	}

	write(fd, buf, 5);

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
	unsigned char destuffed[258];
	unsigned char buf = 0;
	int i = 0, k=0;

	initStateMachineData(&st, SENT_BY_EMISSOR);

	// install handler for alarm signals
	if(alarmSubscribeSignals(alarmHandler)) {
		perror("open_emissor:");
		return -1;
	}

	while (1)
	{
		res += read(fd, &buf, 1);
		if (res > 0)
		{
			printf(" %x ", buf);
			(*st.currentStateFunc)(&st, buf);
			if(k == 2) ns = buf;
			k++;
		}
		if (st.currentState == END)
			break;

		if (st.currentState == DATA){
			dataLink.frame[i] = buf;
			i++;
		}
				
	}
	
	destuffedSize = byteDestuffing(dataLink.frame, i, destuffed);

	//testing////////////////////////////
	/*printf("DB: ");
	for(j = 0; j < destuffedSize; j++)
	{
		printf("%x ", destuffed[j]);
	}
	printf("\n ");*/

	unsigned char calculatedBcc = getBCC(destuffed, destuffedSize-2);
	unsigned char receivedBcc = destuffed[destuffedSize - 1];

	if(calculatedBcc == receivedBcc)
		bccSuccess = 1; //BCC calculated from data is equal to BCC2 received

	extractData(destuffed,buffer,destuffedSize);
	
	res2 = send_R(fd, bccSuccess,ns);

	return res;
}


int llclose(int fd, int status)
{
	printf("\nLLCLOSE\n");
	alarmRaisesCnt = 1, send_flag = 1;

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
