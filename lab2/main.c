#include "parser.h"
#include "ftp.h"

int main(int argc, char** argv) {
	if(argc != 2) {
		printf("Usage todo\n");
		exit(1);
	}

	// parse URL
	struct Address address;
	parseFTPaddress(argv[1], &address);
	printAddress(address);

	// Download file
	Ftp *ftp = ftp_init(address.host, address.user, address.password, address.path, address.filename);
	//Ftp *ftp = ftp_init("test.rebex.net", "demo", "password", "", "KeyGenerator.png");
	//Ftp *ftp = ftp_init("ftp.up.pt", "lol", "ok", "pub/ubuntu/", "ls-lRsjbdsdudb.gz");
	//Ftp ftp = ftp_init("speedtest.tele2.net", NULL, NULL, NULL, "512KB.zip");
	
	int sockfd = ftp_connectToServer(ftp), sockfd_data;

	if(sockfd < 0)
		exit(2);
	printf("\n################# AUTH #################\n");
	if(ftp_authenticateUser(ftp, sockfd))
		exit(3);
	printf("\n################# CHANGING TO PASSIVE MODE #################\n");
	if(ftp_sendPassiveCommand(ftp, sockfd, &sockfd_data))
		exit(4);
	printf("\n################# CHANGING DIRECTORY #################\n");
	if(ftp_changeDirectoryCommand(ftp, sockfd))
		exit(5);
	printf("\n################# DOWNLOADING FILE #################\n");
	if(ftp_sendRetrieveCommand(ftp, sockfd, sockfd_data))
		exit(6);
	

	close(sockfd);
	close(sockfd_data);

	free(ftp);
	return 0;
}