#include "dataLink.h"

#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int send_flag=1, conta=1;

int llopen(int port, int f){
   int fd, i;
    struct termios oldtio,newtio;
    char * portName;

    if(port==1) portName = "/dev/ttyS1";
    else if (port==0 )portName = "/dev/ttyS0";
    else return -1;
    
   fd = open(portName, O_RDWR | O_NOCTTY );
    if (fd <0) {;return -1; }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;


  /*
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
    leitura do(s) pr�ximo(s) caracter(es)
  */

    tcflush(fd, TCIOFLUSH);


    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("input");
      exit(-1);
    }

    printf("New termios structure set\n");

	if(f == EMISSOR_FLAG) open_emissor(fd);
	else if(f == RECEIVER_FLAG) open_receiver(fd);

    return fd;
}


void atende(){
	printf("alarme # %d\n", conta);
	send_flag=1;
	conta++;
}


void open_emissor(int fd){
  int res;
  unsigned char teste;

   stateMachine st;
   st.currentState = START;
   st.currentStateFunc = &stateStart;

   struct sigaction act;
	act.sa_handler = atende;

  sigaction(SIGALRM, &act,NULL);  

  while(conta < 4){
    if(send_flag){

	printf("writing message\n");
        send_SET(fd);

        alarm(3);                 // activa alarme de 3s
	printf("sent alarm\n");
        send_flag=0;
    }

    while(1){
      	res = read(fd,&teste,1);
        printf("%x\n", teste);
        (*st.currentStateFunc)(&st, teste);
        if(st.currentState == END || send_flag) break;
    }

     if(st.currentState == END) return;
  }
  printf("Vou terminar.\n");
}


void open_receiver(int fd){
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
		}
		// update the state machine
		(*st.currentStateFunc)(&st, frame);

		// add frame to packet
		packet[i++] = frame;
	}

	// just for debug
	printf("Recieved\n");

	// display the received packet in hexadecimal
	unsigned char j;
	for (j = 0; j < i; j++) {
		printf("frame %d:%x\n", j, packet[j]);
	}

	// acknowledge the received SET packet and send back UA
	send_UA(fd);
}


int llclose(int fd){
  return 1;
}
