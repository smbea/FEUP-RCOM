//State Machine SET
//WIP
#include "stateMachine.h"
#include <stdio.h>
#include <unistd.h>




int initStateMachine(stateMachine *st, unsigned char r_e_char, unsigned char type) {
	st->currentState = START;
	st->currentStateFunc = &stateStart;
	st->index = 0;

	currentA = r_e_char ;

	currentType = type;

	bccCheck = 0;

	

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
	} else if(input == currentA) {
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

		//printf("\nEXPECTED: %x INPUT: %x \n",currentType,input);

	if(input == currentType) {
		printf("Transitioned to c_rcv state\n");
		st->currentState = C_RCV;
		st->currentStateFunc = &stateProtection;
	} else if(input == FLAG) {
		// go back to flag state
		printf("Got FLAG, back to flag state state\n");
		st->currentState = FLAG;
		st->currentStateFunc = &stateFlag;
	}
	//The packet was reject no use in continuing state machine, back to start
	else if(input == REJ0 || input == REJ1){
		printf("Packet rejected\n");
		st->currentState = START;
		st->currentStateFunc = &stateStart;
		return 1;
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
	} else if (input == (currentA^currentType)) {

			printf("Transitioned to BCC state\n");
			st->currentState = BCC;
			st->currentStateFunc = stateBCC;
	} else {
		st->currentState = START;
		st->currentStateFunc = stateStart;
	}

	return 0;
}

int stateBCC(stateMachine *st, byte input) {
	if(st->currentState != BCC)
		return -1;

	if(currentType == UA || currentType == SET || currentType == DISC){
		if(input == FLAG) st->currentState = END;
		else {
		st->currentState = START;
		st->currentStateFunc = stateStart;
		}
	}else{
		st->currentState = DATA;
		st->currentStateFunc = stateDATA;
	}
}



int stateDATA(stateMachine *st, byte input) {
	
	unsigned char temp;
	
	if(st->currentState != DATA)
		return -1;

	if(input == FLAG){
		st->currentState = END;
		return 1; //Reached flag wihtout finding BCC -> there was an error(REJ)
	}

	/*if(bccCheck == input){ //found BCC -> no errors detected
		st->currentState = FLAG_END;
		st->currentStateFunc = stateEND_FLAG;
	}
	else
	{
		temp = destuffByte(input, st->prev);
		st->prev = input;
		if(temp != NULL){
			bccCheck = bccCheck ^ temp;
			st->message[st->index++] = temp;
		}
	}*/
	return 0;
}

byte destuffByte(byte input, byte prevInput)
{
	if(input == ESCAPE)
		return NULL;
	if(prevInput != ESCAPE)
		return input;
	else if(input == (FLAG ^ 0x20))
		return FLAG;
	else if(input == (ESCAPE ^ 0x20))
		return ESCAPE;
}

int stateEND_FLAG(stateMachine *st, byte input){
	
	if(st->currentState != FLAG_END)
		return -1;

	if(input == FLAG)
	{
		st->currentState = END;
		return 0;
	}
	else
		return -1;
}