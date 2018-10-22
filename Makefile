all: clean stMachine
	gcc -Wall -o dataLink.o dataLink.c stateMachine.o

stMachine:
	gcc -Wall -c stateMachine.c

clean:
		rm -rf *.o
