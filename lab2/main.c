#include "parser.h"
#include "ftp.h"

int main(int argc, char** argv) {
	if(argc != 2) {
		printf("Usage todo\n");
	}
	struct Address address;
	parseFTPaddress(argv[1], &address);
	printAddress(address);
}
