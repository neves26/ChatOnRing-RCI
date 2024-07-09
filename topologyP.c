#include <netinet/in.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "topologyP.h"
#include "userInterface.h"
#include "utils.h"
#include "routingTable.h"

/*************************************************************************************************
*   Function: initNode - initialize a node
*************************************************************************************************/
void initNode(node *no){
	no->id[0]='\0';
	memset(no->buffer, 0, MAX_BUFFER_SIZE);
	no->fd=-1;
	memset(no->ip, 0, sizeof(no->ip));
	memset(no->port,0, sizeof(no->port));
	return;
}

/*************************************************************************************************
*   Function: initNodeApp - initializes structure that has app state
*************************************************************************************************/
void initNodeApp(NodeApp *app, char *hostIP, char *hostTCP){
	app->ring[0] = '\0';
	app->tempfd = -1;
	initNode(&app->ss);
	initNode(&app->adj[SELF]);
	strcpy(app->adj[SELF].ip, hostIP);
	strcpy(app->adj[SELF].port, hostTCP);
	memset(app->msgChat, 0, 256);
	memset(app->buffer,0, MAX_BUFFER_SIZE);
	app->numNo=FIX_COLUMN;
	for(int i=1; i<MAX_NODES; i++){
		initNode(&(app->adj[i]));
	}
	return;
}

/*************************************************************************************************
*   Function: copyNode - Copies the info from node src to dest
*************************************************************************************************/
void copyNode(node *dest, node *src) {
	strcpy(dest->id, src->id);
	strcpy(dest->buffer, src->buffer);
	dest->fd = src->fd;
	return;
}

/*************************************************************************************************
*   Function: chord - Inits a chord to another node in the ring
*************************************************************************************************/
int chord(NodeApp *app) {
	if(chooseIdChord(app) == 1) {
		fprintf(stderr, "Error couldn't add chord\n");
		return 1;
	}
	sendChordMsg(app);
	return 0;
}

/*************************************************************************************************
*   Function: chooseIdChord - Checks if already have initiated my chord
*   If not, then initializes chord with 1st node that isnt a suc or pred
*************************************************************************************************/
int chooseIdChord(NodeApp * app) {
	char msg[128], id[3], ip[16], port[6];
	char *ptr;
	int i = 0, flag = 1;
	node * chord = &app->adj[3];
	sprintf(msg, "NODES %s", app->ring);
	UdpSocket(app, msg);
	ptr = app->buffer;

	if(app->adj[CHRD].id[0] != '\0') {
		fprintf(stderr, "Already have chord with node %s\n", app->adj[3].id);
		return 1;
	}

	if(strncmp(ptr, "NODESLIST ", 10) == 0) {
		// now buffer points to the first '\n'
		ptr= strchr(ptr, '\n');
		ptr++;

		while (sscanf(ptr, "%2s %15s %5s", id, ip, port) == 3 && flag == 1) {
			flag = 0;
			// TODO compare in that vector if(strncmp(id,, ));
			for (i = 0; i < 3; i++) {
				if(strncmp(id, app->adj[i].id, 2) == 0) {
					flag = 1;
					break;
				}
			}
			if(!flag) {
				strcpy(chord->id, id);
				strcpy(chord->ip, ip);
				strcpy(chord->port, port);
				fprintf(stderr, "Chord: %s %s %s\n", app->adj[CHRD].id, app->adj[CHRD].ip, app->adj[CHRD].port);
			}

			ptr = strchr(ptr, '\n'); // Move to the next line
			if (ptr == NULL) break;  // No more lines
			ptr++;  // Move to character after \n
		}
	}


	return flag;
}

/*************************************************************************************************
*   Function: sendEntryMsg - Gets fd to a server and sends ENTRY msg
*************************************************************************************************/
int sendEntryMsg(NodeApp *app, node *no, char* ip, char *port){
	char msgSent[MAX_READ];
	memset(msgSent, 0, MAX_READ);
	int nSent = 0;

	sprintf(msgSent, "ENTRY %s %s %s\n", app->adj[SELF].id, app->adj[SELF].ip, app->adj[SELF].port);
	no->fd = createClientTcpSocket(no, ip , port);
	if(no->fd == -1)
		closeProgram(app);
	nSent = sendMessage(no->fd, msgSent);
	return nSent;
}

/*************************************************************************************************
*   Function: sendSuccMsg - Sends SUCC msg to my predecessor
*************************************************************************************************/
int sendSuccMsg(NodeApp *app, char *id, char *ip, char *port) {
	char msgSent[MAX_READ];
	int nSent = 0;
	memset(msgSent, 0,  MAX_READ);
	sprintf(msgSent, "SUCC %s %s %s\n", id, ip, port);
	nSent = sendMessage(app->adj[PRE].fd, msgSent);
	if(nSent == -1)
		closeProgram(app);
	return nSent;
}


/*************************************************************************************************
*   Function: sendPredMsg - sends PRED msg to my successor
*************************************************************************************************/
int sendPredMsg(NodeApp *app, char *ip, char *port) {
	char msgSent[MAX_READ];
	int nSent = 0;
	memset(msgSent, 0,  MAX_READ);
	sprintf(msgSent, "PRED %s\n", app->adj[SELF].id);
	app->adj[SUC].fd = createClientTcpSocket(&app->adj[SUC], ip, port);
	if(app->adj[SUC].fd <= 0)
		closeProgram(app);
	nSent = sendMessage(app->adj[SUC].fd, msgSent);
	if(nSent == -1)
		closeProgram(app);
  
	return nSent;
}

/*************************************************************************************************
*   Function: sendChordMsg - Gets fd to my chord, then sends CHORD msg
*************************************************************************************************/
int sendChordMsg(NodeApp *app) {
	char msgSent[MAX_READ];
	node * chord = &app->adj[3];
	int nSent = 0;

	sprintf(msgSent, "CHORD %s\n", app->adj[SELF].id);
	chord->fd = createClientTcpSocket(chord, chord->ip, chord->port);
	if(chord->fd <= 0)
		closeProgram(app);
	nSent = sendMessage(chord->fd, msgSent);
	if(nSent == -1)
		closeProgram(app);
	return 0;
}

/*************************************************************************************************
*   Function: createClientTcpSocket - creates client type TCP socket and returns its fd
*************************************************************************************************/
int createClientTcpSocket(node *ptr, const char* ip, const char* port) {
	struct addrinfo hints, *res;
	int fd, errcode;
	ssize_t n;

	// Creates TCP socket
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1) {
		fprintf(stderr, "Error getting fd client TCP\n");
		return -1;
	}
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	errcode = getaddrinfo(ip, port, &hints, &res);
	if(errcode != 0){
		fprintf(stderr, "Error getting fd in createClientTcpSocket\n");
		return -1;
	}

	n = connect(fd, res->ai_addr, res->ai_addrlen);
	if(n == -1 || fd <= 0) {
		fprintf(stderr, "Error connecting TCP, fd = %d\n", fd);
		return -1;
	}

	freeaddrinfo(res);
	return fd;
}

/*************************************************************************************************
*   Function: createServerTcpSocket - Creates TCP server type socket and returns its fd
*************************************************************************************************/
int createServerTcpSocket(node* ptr, const char *port) {
	struct addrinfo hints, *res;
	int fd, errcode;
	ssize_t n;
	// Creates TCP socket
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1) {
		fprintf(stderr, "Error getting fd in creaete server tcp socket\n");
		return -1;
	}

	memset(&hints, 0, sizeof (hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	// DNS lookup
	errcode = getaddrinfo(NULL, port, &hints, &res);
	if(errcode != 0){
		perror("Error getting dns");
		return -1;
	}

	n = bind(fd, res->ai_addr, res->ai_addrlen);
	if(n == -1){
		perror("Error binding socket");
		return -1;
	}

	if(listen(fd, 5) == -1) {
		fprintf(stderr, "Error listening in createClientTcpSocket\n");
		return -1;
	}

	freeaddrinfo(res);
	return fd;
}

/*************************************************************************************************
*   Function: sendMessage - Sends message to node with socket fd
*************************************************************************************************/
int sendMessage(int fd, char *buffer)
{
	ssize_t n = -1;
	n = write(fd, buffer, strlen(buffer));
	if(n == -1){
		fprintf(stderr, "Error writing message with TCP\n");
		return -1;
	}
	return n;
}

/*************************************************************************************************
*   Function: readMessage - Reads message from node with socket fd
*************************************************************************************************/
int readMessage(int fd, char *buffer, int sizeBuffer)
{
	ssize_t n = -1;
	n = read(fd, buffer, sizeBuffer);{
		if(n == -1){
			fprintf(stderr, "Error reading TCP message\n");
		}
	}
	return n;
}

/*************************************************************************************************
*   Function: UdpSocket - send msg to node server
*************************************************************************************************/
int UdpSocket(NodeApp *app, char *msg){
	int fd, errcode;
	ssize_t n;
	struct addrinfo hints, *res;
	struct sockaddr_in addr;
	socklen_t addrlen;
	char buffer[UDP_BUFFER_SIZE];

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(fd==-1) {
		fprintf(stderr, "Error getting fd for UDP socket\n");
		closeProgram(app);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	// this block of code makes it so that if it doesnt
	// receive message for 1 seconds, the program continues
	struct timeval timeout;
	timeout.tv_sec = 5; // Timeout in seconds
	timeout.tv_usec = 0;
	setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

	errcode = getaddrinfo(app->ipUDP, app->portUDP, &hints, &res);
	if(errcode == -1) {
		perror("Error getaddrinfo UDP");
		closeProgram(app);
	}
	n=sendto(fd, msg, strlen(msg), 0, res->ai_addr, res->ai_addrlen);
	if(n==-1) {
		fprintf(stderr, "Error sending message to server nodes\n");
		closeProgram(app);
	} 
	addrlen=sizeof(addr);

	n = 0;
	n=recvfrom(fd, buffer, UDP_BUFFER_SIZE, 0, (struct sockaddr*)&addr, &addrlen);
	if(n == -1) {
		fprintf(stderr, "Erro sending message to the server of nodes");
		closeProgram(app);
	} else if(n == 0) {
		fprintf(stderr, "Couldn't send message to server of nodes. Trying again before exiting program\n");
		n = sendto(fd, msg, strlen(msg), 0, res->ai_addr, res->ai_addrlen);
		if(n == -1 || n == 0) {
			fprintf(stderr, "Error again... closing...\n");
			closeProgram(app);
		}
	} else{
		buffer[n]='\0';
	}

	strcpy(app->buffer, buffer);
	freeaddrinfo(res);
	close(fd);

	return 0;
}

/*************************************************************************************************
*   Function: parseUdpBuffer - handles msgs received from server of nodes
*************************************************************************************************/
void parseUdpBuffer(char *buffer, NodeApp *app) {
	char id[3], ip[16], port[6];
	int nodesID[16];
	int numNodes = 0, idIntegertmp = 0;
	char msg[UDP_BUFFER_SIZE];
	memset(msg, 0, sizeof(msg));

	idIntegertmp = atoi(app->adj[SELF].id);

	if(sscanf(buffer, "NODESLIST %s", app->ring)==1) {

		// now buffer points to the first '\n'
		buffer = strchr(buffer, '\n');
		buffer++;

		while (sscanf(buffer, "%2s %15s %5s", id, ip, port) == 3) {
			nodesID[numNodes] = atoi(id);
			numNodes++;

			if(numNodes == 1 && app->adj[SUC].id[0] == '\0') {
				updateSuc(app, id, ip, port);
			}

			buffer = strchr(buffer, '\n'); // Move to the next line
			if (buffer == NULL) break;  // No more lines
			buffer++;  // Move to character after \n
		}

		// there are 0 nodes in the ring
		if(numNodes == 0) {
			updateSuc(app, app->adj[SELF].id, app->adj[SELF].ip, app->adj[SELF].port);
			updatePred(app, app->adj[SELF].id);
			updateSecSuc(app, app->adj[SELF].id, app->adj[SELF].ip, app->adj[SELF].port);
			registerRing(app);
		} else {
			chooseId(numNodes, idIntegertmp, nodesID, app);
		}
	} else if(strncmp(buffer,"OKREG", 5) ==0){
		//printf("Receive OKREG\n");
		return;
	} else if(strncmp(buffer,"OKUNREG", 6) ==0){
		//printf("Receive OKUNREG\n");

		return;
	}
	return;
}

/*************************************************************************************************
*   Function: chooseId - if id pickec in join invalid, picks valid id from NODESLIST msg to be its new id
*************************************************************************************************/
void chooseId(int numNodes, int currId, int *nodesID , NodeApp *app) {
	bool sameID = false;
	int i = 0;

	for(i = 0; i < numNodes; i++){
		if(currId == nodesID[i]){
			sameID = true;
		}
	}

	if(sameID){
		for(i = 0; i < 100; i++){
			sameID = false;

			for(int j = 0; j < numNodes; j++){
				if(i == nodesID[j]){
					sameID = true;
					break;
				}
			}
			if(!sameID){
				app->adj[SELF].id[0] = (i / 10) + '0';
				app->adj[SELF].id[1] = (i % 10) + '0'; 
				return;
			}
		}
	}
	return;
}

/*************************************************************************************************
*   Function: registerRing - Registers node in server of nodes
*************************************************************************************************/
void registerRing(NodeApp *app)
{
	sprintf(app->buffer, "REG %s %s %s %s", app->ring, app->adj[SELF].id, app->adj[SELF].ip, app->adj[SELF].port);
	UdpSocket(app, app->buffer);
	parseUdpBuffer(app->buffer, app);
	initRoutTab(&app->tab);

	return;
}

/*************************************************************************************************
*   Function: updateSuc - Updates information of my successor in array of nodes
*************************************************************************************************/
void updateSuc(NodeApp *app, char *id, char *ip, char *port){
	strcpy(app->adj[SUC].id, id);
	strcpy(app->adj[SUC].ip, ip);
	strcpy(app->adj[SUC].port, port);
	return;
}

/*************************************************************************************************
*   Function: updateSecSuc - Updates information of my 2nd suc in main structure
*************************************************************************************************/
void updateSecSuc(NodeApp *app, char *id, char *ip, char *port){
	strcpy(app->ss.id, id);
	strcpy(app->ss.ip, ip);
	strcpy(app->ss.port, port);
	return;
}

/*************************************************************************************************
*   Function: updatePred - Updates information of my pred in array of nodes
*************************************************************************************************/
void updatePred(NodeApp *app, char *id){
	strcpy(app->adj[PRE].id, id);
	return;
}

/*************************************************************************************************
*   Function: cpstr - Copies string src to dest string character by character starting from dest[start]
*************************************************************************************************/
int cpstr(char *dest, char *src, int start) {
	int i = 0, cnt = 0;
	char *ptr = src;
	for(i = start; *ptr != '\n'; i++) {
		dest[i] = *ptr;
		ptr++; cnt++;
	}
	return cnt;
}

