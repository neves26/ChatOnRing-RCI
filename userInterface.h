#ifndef USERINTERFACE_H_
#define USERINTERFACE_H_

#include "topologyP.h"
#include "routingTable.h"

void readStdin(NodeApp *, char *);
void readClient(NodeApp * , node *, int);
void readServer(NodeApp *, node *);

int isMsgValid(char *);
void showTopology(NodeApp *);
void freeNode(node *);
void leave(NodeApp *);
void closeProgram(NodeApp *);
void handleChat(NodeApp *, char *);
void checkForConnection(NodeApp *, char *, char *);
void handleMessage(NodeApp *, char *);

void join(NodeApp* , char *, char * );
void djoin(NodeApp *, char *id, char *succid, char *succIP, char *succTCP);
int handleEntryMsg(NodeApp *, node *, char *, int );
int handleSuccMsg(NodeApp *, char *);
int handleChordMsg(NodeApp * , char *, int );
void handleSucLeave(NodeApp *, int );
int insideRing(NodeApp *);

#endif // !INTERFACE_USER_
