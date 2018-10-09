//State Machine SET
//WIP

#define FLAG 0x7E

typedef unsigned char byte;

// Enumerator with all possible machine states
typedef enum st {
	START, FLAG_RCV, A_RCV, C_RCV, BCC_RCV, END
} State;

// Possible values for the field Address
enum AddressField {
	SENT_BY_EMISSOR = 0x03,
	SENT_BY_RECEPTOR = 0x01
};

// Possible values for the control field. Not complete tho..
enum ControlField {
	SET = 0x03,
	DISC = 0x0B,
	UA = 0x07
};

// Struct that represents the machine in some instant
// It holds the current state
// and a function pointer that processes input for that state
typedef struct stateMachine {
	State currentState;
	int (*currentStateFunc)(stateMachine, byte);
} stateMachine;


struct tram{
	int firstFlag;
	int messageType;
	char header;
};


int main()
{
	// the current state of the machine
	State currentState;
	// the function that processes the current state
	int (*currentStateFunc)(State, byte) = &stateStart;
	int flag1;
	char c;
	struct tram alpha;
	int tempByte = 0;
	
	while(1)
	{
		do //check flag
		{ 
			tempByte = readByte();
		}while(tempByte != alpha.firstFlag)
	
		do //check message type
		{ 
			tempByte = readByte();
		}while(tempByte != 3 && tempByte != 1 && tempByte != alpha.firstFlag)
	
		if(tempByte == 3)
			alpha.messageType = 0;
		else if(tempByte == 1)
			alpha.messageType = 1;
		else
			continue;
	
		do
		{
			tempByte = readByte();
		}while(tempByte != 0x02 && tempByte != 0x0B && tempByte != 0x07 && /*!*/ tempByte != 0x05 && tempByte != 0x01 && tempByte != alpha.firstFlag)
		
		switch(tempByte)
		{
			case 0x02:
				alpha.header = 'S';
				break;
			case 0x0B:
				alpha.header = 'D';
				break;
			case 0x07:
				alpha.header = 'U';
				break;
			case 0x05:
				alpha.header = 'R';
				break;
			case 0x01:
				alpha.header = 'J';
				break;
			default:
				continue;
		}
		break;
	}	
}


/**
 * Processes the input when the machine is in the Start state
 */
int stateStart(stateMachine *st, byte input) {
	if(st->currentState != START)
		return -1; // unexpected machine state
	
	// if the input matches 
	if(input == FLAG) {
		// transitate the state to FLAG_RCV and update function
		st->currentState == FLAG_RCV;
		st->currentStateFunc = &stateFlag;
		return 0;
	} else {
		// stays on Start state
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
		st->currentState = A_RCV;
		st->currentStateFunc = &stateAddress;
		return 0;
	} else {
		// unknown input, transitate to Start state
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
		st->currentState = BCC_RCV;
		st->currentStateFunc = &stateProtection;
		return 0;
	} else if(input == FLAG) {
		// go back to flag state
		st->currentState = FLAG;
		st->currentStateFunc = &stateFlag;
		return 0;
	} else {
		// unknown input
		st->currentState = START;
		st->currentStateFunc = &stateStart;
	}
}

int stateProtection(stateMachine *st, byte input) {
	if(st->currentState != C_RCV)
		return -1;
	
	if(input == FLAG) {
		st->currentState = FLAG;
		st->currentStateFunc = stateFlag;
	} else if (input == SENT_BY_EMISSOR^SET) {
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
	}
}