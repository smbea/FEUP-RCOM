make:
	make clean
	gcc -Wall -o dataLink.o dataLink.c stateMachine.c

clean:
	rm -rf *.o
