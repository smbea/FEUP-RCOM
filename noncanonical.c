/*Non-Canonical Input Processing*/

#include "stateMachine.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define RECEIVER_FLAG 0

volatile int STOP = FALSE;

int main(int argc, char **argv) {

	int fd;
	struct termios oldtio, newtio;

	/**
	 * Validate arguments
	 */
	if ((argc < 2) || ((strcmp("/dev/ttyS0", argv[1]) != 0) && (strcmp("/dev/ttyS1", argv[1]) != 0))) {
		printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
		exit(1);
	}

	/**
	 * Open serial port device for reading and writing and not as controlling tty 
	 * because we don't want to get killed if linenoise sends CTRL-C.
	 */

	fd = open(argv[1], O_RDWR | O_NOCTTY);
	if (fd < 0) {
		perror(argv[1]);
		exit(-1);
	}

	if (tcgetattr(fd, &oldtio) == -1) { /* save current port settings */
		perror("tcgetattr");
		exit(-1);
	}

	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;

	/* set input mode (non-canonical, no echo,...) */
	newtio.c_lflag = 0;

	/*
		VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
		leitura do(s) pr�ximo(s) caracter(es)
	*/
	newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
	newtio.c_cc[VMIN] = 1;

	tcflush(fd, TCIOFLUSH);

	if (tcsetattr(fd, TCSANOW, &newtio) == -1) {
		perror("tcsetattr");
		exit(-1);
	}
	printf("New termios structure set\n");

	/**
	 * Initialize state machine and receive packets
	 */
	stateMachine st;
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
