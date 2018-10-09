//State Machine SET
//WIP
#include "stateMachine.h"
#define FLAG 0x7E

struct tram{
	int firstFlag;
	int messageType;
	char header;
};

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
		return 0;
	} else {
		// stays on Start state
		printf("Staying in start state\n");
		return 0;
	}
}

int stateFlag(stateMachine *st, byte input) {
	if(st->currentState != FLAG_RCV)
		return -1;

	if(input == FLAG) {
		// stays on the Flag state
		return 0;
	} else if(input == SENT_BY_EMISSOR || input == SENT_BY_RECEPTOR) {
		printf("Transitioned to a_rcv state\n");
		st->currentState = A_RCV;
		st->currentStateFunc = &stateAddress;
		return 0;
	} else {
		// unknown input, transitate to Start state
		printf("Unknown input, back to start\n");
		st->currentState = START;
		st->currentStateFunc = &stateStart;
		return 0;
	}
}

int stateAddress(stateMachine *st, byte input) {
	if(st->currentState != A_RCV)
		return -1;
	
	if(input == SET || input == DISC || input == UA) {
		// do something I guess...
		printf("Transitioned to c_rcv state\n");
		st->currentState = C_RCV;
		st->currentStateFunc = &stateProtection;
		return 0;
	} else if(input == FLAG) {
		// go back to flag state
		printf("Got FLAG, back to flag state state\n");
		st->currentState = FLAG;
		st->currentStateFunc = &stateFlag;
		return 0;
	} else {
		// unknown input
		printf("Unknown input, back to start\n");
		st->currentState = START;
		st->currentStateFunc = &stateStart;
	}
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
	} else {
		st->currentState = START;
		st->currentStateFunc = stateStart;
	}
}

int stateBCC(stateMachine *st, byte input) {
	if(st->currentState != BCC_RCV)
		return -1;

	if(input == FLAG) {
		printf("end\n");
	} else {
		printf("TODO\n");
	}
}
