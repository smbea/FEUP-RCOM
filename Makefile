all: emitter receiver

stMachine: 
	gcc -Wall -Wextra -c stateMachine.c

receiver: stMachine
	gcc -Wall -Wextra stateMachine.o noncanonical.c -o noncanonical

emitter: stMachine
	gcc -Wall -Wextra stateMachine.o writenoncanonical.c -o writenoncanonical

clean:
	rm *.o
	rm noncanonical
	rm writenoncanonical