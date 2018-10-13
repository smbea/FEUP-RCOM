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
#include "dataLink.h"

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP = FALSE;

int main(int argc, char **argv) {

	int fd, c, res;
	struct termios oldtio, newtio;
	int port = 0;

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

	/*
		O ciclo WHILE deve ser alterado de modo a respeitar o indicado no gui�o
	*/
	tcsetattr(fd, TCSANOW, &oldtio);
	close(fd);
	return 0;
}
