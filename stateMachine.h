#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H
typedef unsigned char byte;
#define FLAG 0x7E

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
	int (*currentStateFunc)(void*, byte);
} stateMachine;

/**
 * @brief Initializes the state machine
 *
 * @param st The state machine to be intialized
 * @return int
 */
int initStateMachine(stateMachine *st);

int stateStart(stateMachine *st, byte input);
int stateFlag(stateMachine *st, byte input);
int stateAddress(stateMachine *st, byte input);
int stateProtection(stateMachine *st, byte input);
int stateBCC(stateMachine *st, byte input);
#endif
