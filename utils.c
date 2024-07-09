#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdbool.h>
#include <stdlib.h>


/*************************************************************************************************
*   Function: readStdin - Reads messages from stdin and call corresponding fucntions to handle them
*************************************************************************************************/
void printCommands() {
	printf("\nWelcome! Here is the list of available commands:\n");
	printf("Type \"clear or cls\" to clean the screen\n");
	printf("Type \"(j)oin ring id\" to join a ring with your id\n");
	printf("Type \"direct join (dj) id succid succIP succTCP\" to join directly to a node inside the ring\n");
	printf("Type \"(c)hord\" to create a chord with a node in the ring\n");
	printf("Type \"remove chord (rc)\" to remove a your chord\n");
	printf("Type \"show topology (st)\" to show self, s, ss, selfChord\n");
	printf("Type \"message (m) dest message\" to message dest\n");
	printf("Type \"(l)eave\" to leave the ring\n");
	printf("Type \"e(x)it\" to close the application\n\n");
	return;
}

/*************************************************************************************************
*   Function: checkArgs - checks the validity of the arguments received
*************************************************************************************************/
int checkArgs(char **argv, int argc) {
	if (validateIPv4(argv[1])){
		fprintf(stderr, "Invalid IP format %s\n", argv[1]);
	exit(1);
	}

	if (validatePort(argv[2])){
		fprintf(stderr, "Invalid Port number %s\n", argv[2]);
	exit(1);
	}

	if (argc==3){
		return 1;

	} else if(argc==5) {

		if (validateIPv4(argv[3])){
			fprintf(stderr, "Invalid IP format %s\n", argv[3]);
			exit(1);
		}
		if (validatePort(argv[4])){
			fprintf(stderr, "Invalid Port number %s\n", argv[4]);
			exit(1);
		}
	}else{
		fprintf(stderr, "Usage: ./COR IP TCP <regIP> <regUDP>\n");
	exit(1);
	}

	return 0;
}


/*************************************************************************************************
*   Function: validateIPv4 - checks if the IP is valid with IPv4 standards
*************************************************************************************************/
int validateIPv4(char *addrIP) {
	int len = strlen(addrIP);
	unsigned int bracket[4];
	int hits = 0;

	if(len < 7 || len > 15) {
		return 1;
	}

	hits = sscanf(addrIP, "%u.%u.%u.%u", &bracket[0], &bracket[1], &bracket[2], &bracket[3]);
	if(hits != 4) {
		return 1;
	}

	for (int i = 0; i < 4; i++) {
		if(bracket[i] > 255) {
			return 1;
		}
	}

	return 0;
}

/*************************************************************************************************
*   Function: validatePort - checks if the PORT is valid
*************************************************************************************************/
int validatePort(char *portNumber) {
	int hits = 0;
	int port = 0;

	if(strlen(portNumber) > 5)
		return 1;

	for(int i = 0; portNumber[i] != '\0' && i > 4; i++)	{
		if(portNumber[i] < 48 || portNumber[i] > 57) {
			return 1;
		}
	}


	hits = sscanf(portNumber, "%d", &port);
	if (hits != 1) {
		return 1;
	} 

	if(port < 1024 || port > 65535) {
		return 1;
	} 
	return 0;
}
