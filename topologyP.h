#ifndef TOPOLOGYP_H_
#define TOPOLOGYP_H_

#include <netinet/in.h>
#include <stdbool.h>
#define SELF 0
#define SUC 1
#define PRE 2
#define CHRD 3

#define MAX_BUFFER_SIZE 1024
#define MAX_READ 256
#define UDP_BUFFER_SIZE 128
#define MAX_NODES 16
#define FIX_COLUMN 4

typedef struct Table {
    char ***rout;
    int map[MAX_NODES];
} Table;

/*
typedef struct address {
	char port[6];
	char ip[16];
} address;
*/

typedef struct node {
    char id[3];
    int fd;
    char buffer[MAX_BUFFER_SIZE];
	//address *addr;
    char port[6];
    char ip[16]; 	
	//table *tab;
} node;

typedef struct NodeApp {
    char ring[4];
    node ss;
	node adj[MAX_NODES];
	Table tab;
	int numNo;
    char buffer[MAX_BUFFER_SIZE];
	char msgChat[256];
	char ipUDP[16];
	char portUDP[6];
	int tempfd;
} NodeApp;

int chooseIdChord(NodeApp * );
int chord(NodeApp * );
void removeChord(NodeApp *, char *);
int UdpSocket(NodeApp *, char *);
void parseUdpBuffer(char *, NodeApp * );
void registerRing(NodeApp * );

void initNode(node *);
void initNodeApp(NodeApp *, char * , char*);
void chooseId(int, int, int * , NodeApp *);

int sendEntryMsg(NodeApp *, node *, char* , char *);
int sendSuccMsg(NodeApp *, char *, char *, char *);
int sendPredMsg(NodeApp *app, char *, char *);
int sendChordMsg(NodeApp *);

int createClientTcpSocket(node *, const char *, const char *);
int createServerTcpSocket(node *, const char *);

int readMessage(int , char *, int );
int sendMessage(int , char *);
void init_sockaddr_in(char *ip, char *port, node *ptr);
int nodePort(node *ptr);
char * nodeIP(node *ptr, char *);

bool checkEntryMsg(NodeApp , char *, bool );
void updateSuc(NodeApp *, char *, char *, char *);
void updatePred(NodeApp *, char *);
void updateSecSuc(NodeApp *, char *, char *, char *);

int treatMessagesClient(NodeApp *, char * , int );
int cpstr(char *dest, char *src, int start);

#endif
