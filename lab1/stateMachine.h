#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H
typedef unsigned char byte;
#define FLAG 0x7E
#define ESCAPE 0x7D

// Enumerator with all possible machine states
typedef enum st {
	START, FLAG_RCV, A_RCV, C_RCV, BCC, BCC1, DATA,DESTUFFING, BCC2 ,END, FLAG_END, REJ
} State;


// Possible values for the field Address
enum AddressField {
	SENT_BY_EMISSOR = 0x03,
	SENT_BY_RECEPTOR = 0x01
};

unsigned char currentA;

// Possible values for the control field. Not complete tho..
enum ControlField {
	SET = 0x03,
	DISC = 0x0B,
	UA = 0x07,
	S0 = 0X00,
	S1 = 0X40,
	RR0 = 0x03,
	RR1 = 0x83,
	REJ0 = 0x01,
	REJ1 = 0x81,
};

byte currentType;

byte bccCheck;


// Struct that represents the machine in some instant
// It holds the current state
// and a function pointer that processes input for that state
typedef struct stateMachine {
	State currentState;
	int (*currentStateFunc)(struct stateMachine*, byte);
	char message[255];
	byte prev;
	int index;
} stateMachine;

/**
 * @brief Initializes the state machine
 *
 * @param st The state machine to be intialized
 * @return int
 */
int initStateMachine(stateMachine *st, unsigned char r_e_flag, unsigned char type);
int initStateMachineData(stateMachine *st, unsigned char r_e_char);

int stateStart(stateMachine *st, byte input);
int stateFlag(stateMachine *st, byte input);
int stateAddress(stateMachine *st, byte input);
int stateProtection(stateMachine *st, byte input);
int stateDATA(stateMachine *st, byte input);
int stateBCC1(stateMachine *st, byte input);
int stateBCC(stateMachine *st, byte input);
int stateEND_FLAG(stateMachine *st, byte input);
#endif
