/*Non-Canonical Input Processing*/

#include "dataLink.h"
#include "stateMachine.h"
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define EMISSOR_FLAG 1

#define A 0x03
#define C 0x03
#define BCC A ^ C

volatile int STOP=FALSE;

stateMachine st;
int flag=1, conta=1;

void atende(){
	printf("alarme # %d\n", conta);
	flag=1;
	conta++;
}


void writemessage(int fd){
    int res;
    
    unsigned char buf[6];
    buf[0] = FLAG;
    buf[1] = A;
    buf[2] = C;
    buf[3] = BCC;
    buf[4] = FLAG;
    buf[5] = 0;

    res = write(fd,buf,sizeof(buf));
    printf("%d bytes written\n", res);
}


void communicateWithReceptor(int fd){
  int res;
  unsigned char teste;

   stateMachine st;
   st.currentState = START;
   st.currentStateFunc = &stateStart;

   struct sigaction act;
	act.sa_handler = atende;

  sigaction(SIGALRM, &act,NULL);  

  while(conta < 4){
    if(flag){

	printf("writing message\n");
        writemessage(fd);

        alarm(3);                 // activa alarme de 3s
	printf("sent alarm\n");
        flag=0;
    }

    while(1){
      	res = read(fd,&teste,1);
        printf("%x\n", teste);
        (*st.currentStateFunc)(&st, teste);
        if(st.currentState == END || flag) break;
    }

     if(st.currentState == END) return;
  }
  printf("Vou terminar.\n");
}


int main(int argc, char** argv)
{
    int fd;
    struct termios oldtio;
    int port=0;

    if ( (argc < 2) ||
         ((strcmp("/dev/ttyS0", argv[1])!=0) &&
          (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }else if(strcmp("/dev/ttyS0", argv[1])==0) port = COM1;
    else if(strcmp("/dev/ttyS1", argv[1])==0) port = COM2;


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */

    fd = llopen(port,EMISSOR_FLAG);
   
    communicateWithReceptor(fd);

    fflush(NULL);


    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }


    close(fd);
    return 0;
}
