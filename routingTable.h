#ifndef ROUTINGTABLE_H_
#define ROUTINGTABLE_H_
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

#include "topologyP.h"


//function(NodeApp *app  )
//node *no=app->adj;

//table* createTable(char*, char *);
void initNode(node *);
void updatetNode(node *, char *, char *, int);   

void initApp(NodeApp *);
void initRoutTab(Table *);
void freeApp(NodeApp*);
void freeRoutTab(NodeApp *);

void addNeighbour(NodeApp *, char *, int );
void addChord(NodeApp *, char *, int );

int firstPosFree(NodeApp *);
void changeColSize(NodeApp *, int );
void delNeighb(NodeApp *, int );
void addChord(NodeApp *, char *, int);
void addNeighb(NodeApp *app, char *id, int fd, int column);

void popRopeInt(NodeApp *);
void popRopeInt2(NodeApp *);

void popRope(NodeApp*);
void popPred(NodeApp *);
void popSucc(NodeApp *);
void popSelf(NodeApp *);

void pShortPath(NodeApp *);
void showForwTable(NodeApp *);
void pRoutTable(NodeApp *);
void showShortPath(NodeApp *, char *);
void showRout(NodeApp *, char *);

int distPath(char *);
void pathVector(char *, int **, int *);
void idChar(int, char*);

// //Necessary function
// void showRoutDest(NodeApp , char *);
// table* findRoutDest(table *, char *);
// void showShortPath(NodeApp, char *);
// void showForwTable(NodeApp);

// void newConnection(NodeApp , char *, int);
// //Delete later
// void PrintAll(NodeApp app);

#endif
