all: emitter receiver

stMachine: 
	gcc -Wall -Wextra -c stateMachine.c

dataLink: 
	gcc -Wall -Wextra -c dataLink.c	

receiver: stMachine dataLink
	gcc -Wall -Wextra stateMachine.o dataLink.o noncanonical.c -o noncanonical.o

emitter: stMachine dataLink
	gcc -Wall -Wextra stateMachine.o dataLink.o writenoncanonical.c -o writenoncanonical.o

clean:
	rm *.o