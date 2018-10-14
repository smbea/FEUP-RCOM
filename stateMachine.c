//State Machine SET
//WIP
#include "stateMachine.h"
#include <stdio.h>
#include <unistd.h>

int initStateMachine(stateMachine *st) {
	st->currentState = START;
	st->currentStateFunc = &stateStart;

	return 0;
}

/**
 * Processes the input when the machine is in the Start state
 */
int stateStart(stateMachine *st, byte input) {
	if(st->currentState != START)
		return -1; // unexpected machine state
	
	// if the input matches 
	if(input == FLAG) {
		printf("Transitioned to flag_rcv state\n");
		// transitate the state to FLAG_RCV and update function
		st->currentState = FLAG_RCV;
		st->currentStateFunc = &stateFlag;
	} else {
		// stays on Start state
		printf("Staying in start state\n");
	}
	return 0;
}

int stateFlag(stateMachine *st, byte input) {
	if(st->currentState != FLAG_RCV)
		return -1;

	if(input == FLAG) {
		printf("Staying in FLAG state");
	} else if(input == SENT_BY_EMISSOR || input == SENT_BY_RECEPTOR) {
		printf("Transitioned to a_rcv state\n");
		st->currentState = A_RCV;
		st->currentStateFunc = &stateAddress;
	} else {
		// unknown input, transitate to Start state
		printf("Unknown input, back to start\n");
		st->currentState = START;
		st->currentStateFunc = &stateStart;
	}

	return 0;
}

int stateAddress(stateMachine *st, byte input) {
	if(st->currentState != A_RCV)
		return -1;
	
	if(input == SET || input == DISC || input == UA) {
		// do something I guess...
		printf("Transitioned to c_rcv state\n");
		st->currentState = C_RCV;
		st->currentStateFunc = &stateProtection;
	} else if(input == FLAG) {
		// go back to flag state
		printf("Got FLAG, back to flag state state\n");
		st->currentState = FLAG;
		st->currentStateFunc = &stateFlag;
	} else {
		// unknown input
		printf("Unknown input, back to start\n");
		st->currentState = START;
		st->currentStateFunc = &stateStart;
	}

	return 0;
}

int stateProtection(stateMachine *st, byte input) {
	if(st->currentState != C_RCV)
		return -1;
	
	if(input == FLAG) {
		printf("Got flag, back to flag state\n");
		st->currentState = FLAG;
		st->currentStateFunc = stateFlag;
	} else if (input == (SENT_BY_EMISSOR^SET)) {
		printf("Transitioned to bcc_rcv state\n");
		st->currentState = BCC_RCV;
		st->currentStateFunc = stateBCC;
	}
	else if (input == (SENT_BY_RECEPTOR^UA)) {
		printf("Transitioned to bcc_rcv state\n");
		st->currentState = BCC_RCV;
		st->currentStateFunc = stateBCC;
	} else {
		st->currentState = START;
		st->currentStateFunc = stateStart;
	}

	return 0;
}

int stateBCC(stateMachine *st, byte input) {
	if(st->currentState != BCC_RCV)
		return -1;

	if(input == FLAG) {
		st->currentState = END;
	} else {
		st->currentState = START;
		st->currentStateFunc = stateStart;
	}

	return 0;
}

/**
 * The code below isn't in any way related with state machine, are just functions 
 * 
 */
int send_SET(int fd) {
	unsigned char buf[5] = {FLAG, SENT_BY_EMISSOR, SET, SENT_BY_EMISSOR ^ SET, FLAG};
	write(fd, buf, 5);
	printf("sent SET packet\n");

	return 0;
}

int send_UA(int fd) {
	// TODO
	unsigned char buf[5] = {FLAG, SENT_BY_RECEPTOR, UA, SENT_BY_RECEPTOR ^ UA, FLAG};
	write(fd, buf, 5);
	printf("sent UA packet\n");

	return 0;
}