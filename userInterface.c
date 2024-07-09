#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "topologyP.h"
#include "userInterface.h"
#include "utils.h"
#include "routingTable.h"

/*************************************************************************************************
*   Function: readStdin - Reads messages from stdin and call corresponding fucntions to handle them
*************************************************************************************************/
void readStdin(NodeApp *app, char *buffer) {
	char ring[4], id[3], succid[3], succIP[16], succTCP[6];
	char msg[MAX_READ];
	memset(msg, 0, MAX_READ);
	int n = -1;

	/*
	showShortPath(&app);
	showForwTable(&app);
	showRout(&app, char *);
	pRoutTable(&app);
	freeRoutTab(&app);
	*/

	if((strncmp(buffer, "join ", 5) == 0) || (strncmp(buffer, "j ", 2) == 0)) {
		if((n = sscanf(buffer, "%*s %3s %2s", ring, id)) != 2) {
			fprintf(stderr, "Error reading from terminal: %s\n", buffer);
			return;
		}
		join(app, ring, id);
	} else if ( (strncmp(buffer, "direct ", 7) == 0) || (strncmp(buffer, "dj ", 3) == 0)) {
		if( (n = sscanf(buffer, "%*s %*s %s %s %s %s", id, succid, succIP, succTCP)) != 4) {
			fprintf(stderr, "Error reading from terminal: %s\n", buffer);
			return;
		}
		djoin(app, id, succid, succIP, succTCP);
	} else if ( strncmp(buffer, "c\n", 2) == 0 || strncmp(buffer, "chord", 5) == 0) {
		if(insideRing(app)) {
			// TODO create chord
		chord(app);
		}
	} else if ( strncmp(buffer, "cls", 3) == 0 || strncmp(buffer, "clear", 5) == 0 ){
		system("clear"); // clears screen
	} else if ( strncmp(buffer, "rc", 2) == 0 || strncmp(buffer, "remove chord", 12) == 0 ){
		if(insideRing(app)) {
			close(app->adj[CHRD].fd);
			delNeighb(app, 3);
		}
	} else if ( strncmp(buffer, "st", 2) == 0 || strncmp(buffer, "show topology", 13) == 0 ){
		showTopology(app);
	} else if ( strncmp(buffer, "sr ", 3) == 0 || strncmp(buffer, "show routing ", 13) == 0 ){
		if(insideRing(app)) {
		}
		// TODO show routing
	} else if ( strncmp(buffer, "sp ", 3) == 0 || strncmp(buffer, "show path ", 10) == 0 ){
		if(insideRing(app)) {
		}
		// TODO show path
	} else if ( strncmp(buffer, "sf", 2) == 0 || strncmp(buffer, "show forwarding", 15) == 0 ){
		if(insideRing(app)) {
		}
		// TODO show forwarding
	} else if ( strncmp(buffer, "m ", 2) == 0 || strncmp(buffer, "message ", 8) == 0 ){
		if(insideRing(app)) {
			handleMessage(app, buffer);
		}
	} else if( strncmp(buffer, "l\n", 2) == 0 || strncmp(buffer, "leave", 5) == 0 ) {
		if(insideRing(app)) {
		leave(app);
		}
	} else if( strncmp(buffer, "x\n", 2) == 0 || strncmp(buffer, "exit", 4) == 0 ) {
		closeProgram(app);
	} else {
		printf("Error: the command \"%s\" is incorrect\n",buffer);
	}
	return;
}

/*************************************************************************************************
*   Function: handleMessage - Handles command message from stdin
*************************************************************************************************/
void handleMessage(NodeApp *app, char *buffer) {
	int nConv = 0;
	char idR[3], chatMsg[128+1], sentMsg[256];

	if( (nConv = sscanf(buffer, "%*s %2s %128[^\n]", idR, chatMsg)) != 2) {
		fprintf(stderr, "Invalid chat message: %s\n", chatMsg);
		return;
	} else if( strcmp(idR, app->adj[SELF].id) == 0) {
		fprintf(stderr, "Can't send messages to myself\n");
	} else {
		sprintf(sentMsg, "CHAT %s %s %s\n", app->adj[SELF].id, idR, chatMsg);
		strcpy(app->msgChat, sentMsg);
		checkForConnection(app, sentMsg, idR);
	}
	return;
}

/*************************************************************************************************
*   Function: checkForConnection - if dest of CHAT msg is connected to me, send CHAT msg to him
*   otherwise send it to successor node
*************************************************************************************************/
void checkForConnection(NodeApp *app, char *chat, char *id) {
	int hasConnection = 0;

	for(int i = 1; i < app->numNo && !hasConnection; i++) {
		if(strcmp(id, app->adj[i].id) == 0) {
			hasConnection = 1;
			fprintf(stderr, "Sending to %s: %s", app->adj[i].id, chat);
			sendMessage(app->adj[i].fd, chat);
			break;
		}
	}
	if(!hasConnection) {
		fprintf(stderr, "Sending to %s: %s", app->adj[SUC].id, chat);
		sendMessage(app->adj[SUC].fd, chat);
	}

	return;
}

/*************************************************************************************************
*   Function: handleChat - handles CHAT message received.
*************************************************************************************************/
void handleChat(NodeApp *app, char *buffer) {
	char idS[3], idR[3], chatMsg[129];
	int nConv = 0;

	if( (nConv = sscanf(buffer, "%*s %3s %3s %128[^\n]", idS, idR, chatMsg)) != 3) {
		fprintf(stderr, "Invalid chat message: %s\n", chatMsg);
		return;
	}
	
	if(strcmp(app->msgChat, buffer) == 0) { // msg circled back, the dest node in not on ring
		fprintf(stderr, "The node %s is not on ring\n", idR);
		return;
	}

	if(strcmp(idR, app->adj[SELF].id) == 0) {
		printf("CHAT from %s: %s\n", idS, chatMsg);
	} else {
		checkForConnection(app, buffer, idR);
	}
	return;
}

/*************************************************************************************************
*   Function: insideRing - checks if currently inside a node
*************************************************************************************************/
int insideRing(NodeApp *app) {
	if(app->ring[0] == '\0') {
		printf("Currently not inside a node\n");
		return 0;
	}
	return 1;
}


/*************************************************************************************************
*   Function: readClient - handles TCP from a node that is my client
*************************************************************************************************/
void readClient(NodeApp * app, node * no, int fd) {
	char id[3];

	if(strncmp(no->buffer, "ENTRY ", 6) == 0) {
		if(isMsgValid(no->buffer) == 0) {
			handleEntryMsg(app, no, no->buffer, fd);
			return;
		} else {
			// TODO handle errors
			return;
		}
	} else if(strncmp(no->buffer, "PRED ", 5) == 0) {
		sscanf(no->buffer, "%*s %2s", id);
		if(app->adj[2].id[0] == '\0') {
			updatePred(app, id);
			app->adj[2].fd = fd;
			sendSuccMsg(app, app->adj[1].id, app->adj[1].ip, app->adj[1].port);
		} else {
			updatePred(app, id);
			app->adj[2].fd = fd;
		}
	} else if(strncmp(no->buffer, "CHORD ", 6) == 0) {
		handleChordMsg(app, no->buffer, fd);
	} else if(strncmp(no->buffer, "CHAT ", 5) == 0) {
		handleChat(app, no->buffer);
	} else {
		return;
	}
	return;
}	

/*************************************************************************************************
*   Function: handleChordMsg - calls fucntion to add chord if its valid
*************************************************************************************************/
int handleChordMsg(NodeApp *app, char *buffer, int fd) {
	char id[3];	
	sscanf(app->adj[0].buffer, "%*s %s", id);
	addChord(app, id, fd);

	return 0;
}

/*************************************************************************************************
*   Function: readServer - handles TCP message from a server node
*************************************************************************************************/
void readServer(NodeApp * app, node * no) {
	if(strncmp(no->buffer, "SUCC ", 5) == 0) {
		if(isMsgValid(no->buffer) == 0)
			handleSuccMsg(app, no->buffer);
		// TODO handle errors
	} else if(strncmp(no->buffer, "ENTRY ", 6) == 0) {
		if(isMsgValid(no->buffer) == 0)
			handleEntryMsg(app, no, no->buffer, no->fd);
		// TODO handle errors
	} else if(strncmp(no->buffer, "CHAT ", 5) == 0) {
		handleChat(app, no->buffer);
	} else {
		fprintf(stderr, "Message received with incorrect format: %s\n", no->buffer);
		// TODO handle errors
	}
	return;
}

/*************************************************************************************************
*   Function: handleSuccMsg - checks if its valid msg and calls funtion to update my 2nd successor 
*   in the main structure
*************************************************************************************************/
int handleSuccMsg(NodeApp *app, char *buffer) {
	char id[3], ip[16], port[6];
	int n = -1;
	if( (n = sscanf(buffer, "%*s %3s %15s %5s", id, ip, port)) != 3) {
		fprintf(stderr, "Error: didnt receive correct SUCC: %s\n", buffer);
		// TODO handle errors
	} else {
		updateSecSuc(app, id, ip, port);
	}

	return 0;
}

/*************************************************************************************************
*   Function: handleEntryMsg - handles ENTRY msg and updates the topology of various nodes
*************************************************************************************************/
int handleEntryMsg(NodeApp *app, node *no, char *buffer, int newfd) {
	char id[3], ip[16], port[16];
	int nSent = 0;
	sscanf(buffer, "%*s %3s %15s %5s", id, ip, port);

	if(strcmp(app->adj[0].id,app->adj[1].id) == 0 && strcmp(app->adj[0].id, app->ss.id) == 0) { // im the only node in ring
		// update node app
		updateSuc(app, id, ip, port);
		// here would send entry message before update pred if more nodes in ring
		updatePred(app, id);
		app->adj[2].fd = newfd;
		sendSuccMsg(app, id, ip, port);
		sendPredMsg(app, ip, port);
	} else if(app->adj[SUC].fd == newfd) { // my succ sent me an entry
		close(app->adj[1].fd);
		updateSecSuc(app, app->adj[1].id, app->adj[1].ip, app->adj[1].port);
		updateSuc(app, id, ip, port);
		sendSuccMsg(app, app->adj[1].id, app->adj[1].ip, app->adj[1].port);
		sendPredMsg(app, app->adj[1].ip, app->adj[1].port);
	} else {
		nSent = sendMessage(app->adj[2].fd, app->adj[0].buffer); // send entry msg back to my pred
		if(nSent == -1)
			closeProgram(app);
		app->tempfd = app->adj[2].fd;
		updatePred(app, id); 
		app->adj[2].fd = newfd;
		sendSuccMsg(app, app->adj[1].id, app->adj[1].ip, app->adj[1].port); // send succ msg to my new pred
	}


	return 0;
}

/*************************************************************************************************
*   Function: isMsgValid - checks if the msg/command is correct
*************************************************************************************************/
int isMsgValid(char *buffer) {
	char id[3], ip[16], port[16];
	int n = -1;

	if( (n = sscanf(buffer, "%*s %3s %15s %5s", id, ip, port) != 3)) {
		fprintf(stderr, "Wrong COMMAND format %s\n", buffer);
		return -1;
	}


	if(validateIPv4(ip)) {
		fprintf(stderr, "Wrong IP format %s\n", buffer);
		return -1;
	}


	if(validatePort(port)) {
		fprintf(stderr, "Wrong PORT format %s\n", buffer);
		return -1;
	}

	return 0;
}

/*************************************************************************************************
*   Function: join - checks if already inside ring then call necessary fucntions to register inside ring
*   and initiate necessary tables and array of nodes
*************************************************************************************************/
void join(NodeApp *app, char *ring, char *id) {
	char msg[MAX_READ];

	if(app->ring[0] != '\0') {
		fprintf(stderr, "Currently inside ring %s\n", app->ring);
		return;
	}

	sprintf(msg, "NODES %s", ring);
	if(UdpSocket(app, msg)) {
		fprintf(stderr, "Error: Couldn't register to node server. Try again in a moment.\n");
		return;
	}

	strcpy(app->ring, ring);
	strcpy(app->adj[0].id, id);

	parseUdpBuffer(app->buffer, app);
	app->adj[0].fd = createServerTcpSocket(&app->adj[0], app->adj[0].port);
	if(app->adj[0].fd == -1) {
		closeProgram(app);
	}

	//we arent the first node in the ring
	if (strcmp(app->adj[0].id, app->adj[1].id)!=0) {
		sendEntryMsg(app, &app->adj[1], app->adj[1].ip, app->adj[1].port);
		registerRing(app);
	}
	return;
}

/*************************************************************************************************
*   Function: djoin - enters the ring without communicating with node server. IP and PORT of
*   the future successor node necessary
*************************************************************************************************/
void djoin(NodeApp *app, char *id, char *succid,
		   char *succIP, char *succTCP) {
	// if Currently in a node, then exit 
	if(app->ring[0] != '\n') {
		printf("Currently already in ring %s\n", app->ring);
		return;
	}

	// djoin doesnt receive the info about the ring,
	// so we choose "061" as a placeholder telling
	// its currently in a ring
	strcpy(app->ring, "061");
	strcpy(app->adj[0].id, id);

	// if id == succid then create ring with 1 node 
	if(strcmp(id, succid) == 0) {
		updateSuc(app, app->adj[0].id, app->adj[0].ip, app->adj[0].port);
		updateSecSuc(app, app->adj[0].id, app->adj[0].ip, app->adj[0].port);
		updatePred(app, app->adj[0].id);
		registerRing(app);
	} else {
		updateSuc(app, succid, succIP, succTCP);
		sendEntryMsg(app, &app->adj[SUC], app->adj[SUC].ip, app->adj[SUC].port);
		registerRing(app);
	}
	return;
}

/*************************************************************************************************
*   Function: showTopology - prints to command line the current topology
*************************************************************************************************/
void showTopology(NodeApp *app)
{
	if(app->ring[0] == '\0'){
		printf("Currently not inside a node\n");
	} else {		
		printf("Node Information:\n");
		printf("ME:\tID - %s\tIP: %s\tPort: %s\tFD: %d\n", app->adj[0].id,  app->adj[0].ip, app->adj[0].port, app->adj[0].fd);

		if(app->adj[SUC].fd != 0){
			printf("S:\tID - %s\tIP: %s\tPort: %s\tFD: %d\n", app->adj[SUC].id, app->adj[SUC].ip, app->adj[SUC].port, app->adj[SUC].fd);
		}
		if(app->ss.fd != 0){
			printf("SS:\tID - %s\tIP: %s\tPort: %s\n", app->ss.id, app->ss.ip, app->ss.port);
		}
		if(app->adj[2].id[0] != '\0'){
			printf("P:\tID - %s\tFD: %d\n", app->adj[2].id, app->adj[2].fd);
		}
		if(app->adj[CHRD].id[0] != '\0'){
			printf("My chord:\tID - %s\tIP: %s\tPort: %s\tFD: %d\n", app->adj[CHRD].id, app->adj[CHRD].ip, app->adj[CHRD].port, app->adj[CHRD].fd);
		}
		for(int i =4; i < app->numNo; i++) {
			if(app->adj[i].id[0] != '\0'){
				printf("C:\tID - %s\tFD: %d\n", app->adj[i].id, app->adj[i].fd);
			}
		}
	}
	return;
}

/*************************************************************************************************
*   Function: handleSucLeave - updates necessary topology when a successor node leaves
*************************************************************************************************/
void handleSucLeave(NodeApp *app, int fd) {
	close(fd);
	if(strcmp(app->adj[SELF].id, app->ss.id) == 0) { // previously 2 nodes in ring
		initNode(&app->adj[PRE]);
		initNode(&app->adj[SUC]);
		updatePred(app, app->adj[SELF].id);
		updateSuc(app, app->adj[SELF].id, app->adj[SELF].ip, app->adj[SELF].port);
	} else {
		initNode(&app->adj[SUC]);
		updateSuc(app, app->ss.id, app->ss.ip, app->ss.port);
		sendSuccMsg(app, app->adj[SUC].id, app->adj[SUC].ip, app->adj[SUC].port);
		sendPredMsg(app, app->adj[SUC].ip, app->adj[SUC].port);
		// updateSecSuc(NodeApp *, char *, char *, char *)
	}

	return;
}


/*************************************************************************************************
*   Function: leave - sends leave msg to node server and frees memory allocated
*************************************************************************************************/
void leave(NodeApp *app) {
	sprintf(app->buffer,"UNREG %s %s", app->ring, app->adj[0].id);
	UdpSocket(app, app->buffer);
	parseUdpBuffer(app->buffer, app);

	app->ring[0] = '\0';
	app->adj[0].id[0] = '\0';

	for(int i = 0; i < app->numNo; i++) {
		if(app->adj[i].fd != -1) {
			close(app->adj[i].fd);
		}
		initNode(&app->adj[i]);
	}

	freeRoutTab(app);
	return;
}

/*************************************************************************************************
*   Function: closeProgram - calls function to leave ring then exits
*************************************************************************************************/
void closeProgram(NodeApp *app) {
	if(insideRing(app)) {
		leave(app);
	}
	exit(0);
}	
