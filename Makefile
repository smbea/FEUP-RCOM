all: clean stMachine
	gcc -Wall dataLink.c stateMachine.o

stMachine:
	gcc -Wall -c stateMachine.c

clean:
		rm -rf *.o
