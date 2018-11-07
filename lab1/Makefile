all: clean stMachine linkLayer
	gcc -Wall -o application.o dataLink.o stateMachine.o application.c -lm

linkLayer:
	gcc -Wall -c dataLink.c

stMachine:
	gcc -Wall -c stateMachine.c

clean:
		rm -rf *.o
