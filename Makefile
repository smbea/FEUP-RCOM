all: emitter receiver

stMachine: 
	gcc -Wall -Wextra -c stateMachine.c

dataLink: 
	gcc -Wall -Wextra -c dataLink.c	

receiver: stMachine
	gcc -Wall -Wextra stateMachine.o dataLink.o noncanonical.c -o noncanonical

emitter: stMachine
	gcc -Wall -Wextra stateMachine.o dataLink.o writenoncanonical.c -o writenoncanonical

clean:
	rm *.o
	rm noncanonical
	rm writenoncanonical