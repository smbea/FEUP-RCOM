/*Non-Canonical Input Processing*/
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include "dataLink.h"

#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define RECEIVER_FLAG 0

volatile int STOP = FALSE;

int main(int argc, char **argv) {

	int fd, port = 0;
	struct termios oldtio;

	/**
	 * Validate arguments
	 */
	if ((argc < 2) || ((strcmp("/dev/ttyS0", argv[1]) != 0) && (strcmp("/dev/ttyS1", argv[1]) != 0))) {
		printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
		exit(1);
	 }else if(strcmp("/dev/ttyS0", argv[1])==0) port = COM1;
    else if(strcmp("/dev/ttyS1", argv[1])==0) port = COM2;

	/**
	 * Open serial port device for reading and writing and not as controlling tty 
	 * because we don't want to get killed if linenoise sends CTRL-C.
	 */

	fd = llopen(port,RECEIVER_FLAG);

	/**
	 * Initialize state machine and receive packets
	 */
	initStateMachine(&st);

	/**
	 * Get input from serial port and process it
	 */
	unsigned char frame;
	unsigned char packet[255];
	unsigned int i = 0;
	while (st.currentState != END) {
		if(read(fd, &frame, 1) != 1) {
			// something went wrong
			perror("Failed to get frame!");
			return -1;
		}
		// update the state machine
		(*st.currentStateFunc)(&st, frame);

		// add frame to packet
		packet[i++] = frame;
		printf("%x\n", packet[i++]);
	}

	// just for debug
	printf("Recieved\n");

	// acknowledge the received SET packet and send back UA
	send_UA(fd);

	/*
		O ciclo WHILE deve ser alterado de modo a respeitar o indicado no gui�o
	*/
	tcsetattr(fd, TCSANOW, &oldtio);
	close(fd);
	return 0;
}
