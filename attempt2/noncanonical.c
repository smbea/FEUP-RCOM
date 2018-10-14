/*Non-Canonical Input Processing*/
#include "stateMachine.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;

int main(int argc, char** argv)
{

    int fd,c, res;
    struct termios oldtio,newtio;
    char buf[255];

    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
  
    
    fd = open(argv[1], O_RDWR | O_NOCTTY);
    if (fd <0) {perror(argv[1]); exit(-1); }

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
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");

	stateMachine st;
	st.currentState = START;
	st.currentStateFunc = &stateStart; 

	unsigned char frame;
	unsigned char packet[255];
	unsigned int i = 0;
	while (st.currentState != END) {
		if (read(fd, &frame, 1) > 0){
		(*st.currentStateFunc)(&st, frame);

		// add frame to packet
		packet[i++] = frame;
		}
		
	}

	printf("received: %X\n", packet);

	send_UA(fd);
	
	/* char message[255];
	int i = 0;
    while (STOP==FALSE) {       // loop for input 
		res = read(fd,buf,1);
		buf[res] = 0;
		message[i] = buf[0];
		i++;
		if(buf[0] == '\0')
			STOP=TRUE;
    }
	
	printf("Recieved:%X\n", message); 
	
	res = write(fd,message,strlen(message)+1);
	*/
	sleep(3);
    	printf("%d bytes written\n", res);



  /* 
    O ciclo WHILE deve ser alterado de modo a respeitar o indicado no gui�o 
  */
    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}

