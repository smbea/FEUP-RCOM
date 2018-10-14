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

struct termios oldtio,newtio;
int send_flag=1, conta=1;

int main(int argc, char** argv){
  int r_e_flag = 0;
  int port, fd;

  if(argc<3){
    printf("Incorrect number of arguments\n");
    exit(1);
  }

  //port
  if(atoi(argv[1])==0) port =  COM1;
  else if(atoi(argv[1])==1) port = COM2;
  else{
    printf("First argument invalid\n");
    exit(1);
  }

//
  if(atoi(argv[2])==0) r_e_flag = EMISSOR_FLAG;
  else if(atoi(argv[2])==1) r_e_flag = RECEIVER_FLAG;
  else{
    printf("Second argument invalid\n");
    exit(1);
  }

  fd = llopen(port,r_e_flag);

  fflush(NULL);


  if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  close(fd);

  return 0;
}


int llopen(int port, int r_e_flag){
  int fd;
  char * portName;

  if(port==COM1) portName = "/dev/ttyS0";
  else if (port==COM2) portName = "/dev/ttyS1";
  else return -1;

  fd = open(portName, O_RDWR | O_NOCTTY );
  if (fd <0) {perror(portName); exit(-1); }

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

  newtio.c_cc[VTIME]    = 1;   /* inter-character timer unused */
  newtio.c_cc[VMIN]     = 0;

  tcflush(fd, TCIOFLUSH);

  if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  printf("New termios structure set\n");

  if(r_e_flag == RECEIVER_FLAG) open_receiver(fd);
  else if(r_e_flag == EMISSOR_FLAG) open_emissor(fd);

  return fd;
}

void open_receiver(int fd){
  int res;

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

	sleep(3);
  printf("%d bytes written\n", res);
}

void atende(int signo){
	printf("alarme # %d\n", conta);
	send_flag=1;
	conta++;
}

void open_emissor(int fd){
  int res;

  st.currentState = START;
  st.currentStateFunc = &stateStart;

  struct sigaction act;
  act.sa_handler = atende;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;

  if (sigaction(SIGALRM, &act,NULL) == -1){
    printf("Error\n");
    exit(-1);
  }

  unsigned char teste;

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
      if (res > 0){
        printf("%x\n", teste);
              (*st.currentStateFunc)(&st, teste);
      }
            if(st.currentState == END || send_flag) break;
        }

     if(st.currentState == END) return;
  }
}
