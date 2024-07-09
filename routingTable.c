#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#define MAX_BUFFER_SIZE 1024
#define MAX_NODES 16
#define FIX_COLUMN 4
#include "routingTable.h"

/* **********************************************************************************************
*   Function: createTable - creates a table cell in our RoutingTable
*************************************************************************************************/
// table* createTable(char id[3], char *path){

//     table* newTable =(table*) malloc(sizeof(table));
//     if (newTable == NULL){
//         printf("Memory allocation failure\n");
//     }
//     strncpy(newTable->id, id, 3);
//     newTable->id[2] = '\0';
//     newTable->next =NULL;
//     newTable->path=strdup(path);
//     if (newTable->path == NULL){
//         printf("Memory allocation failure\n");
//     }
//     return newTable;
// }

/* ***********************************************************************************************
*   Function: initNode - initialize a node
*************************************************************************************************/
/*void initNode(node *no){   
    memset(no->buffer, 0, MAX_BUFFER_SIZE);
    no->id[0]='\0';
    no->fd=-1;
}
*/

/* ***********************************************************************************************
*   Function: updateNode - update a node
*************************************************************************************************/
void updatetNode(node *no, char *id, char *buffer, int fd){   
    strcpy(no->id, id);
    strcpy(no->buffer, buffer);
    no->fd=fd;
}
/* ***********************************************************************************************
*   Function: initApp - initialize the main application
*************************************************************************************************/
void initApp(NodeApp *app){
    
    app->numNo=FIX_COLUMN;
    for(int i=0; i<MAX_NODES; i++){
        initNode(&(app->adj[i]));
    }
    initRoutTab(&(app->tab));
}

/* ***********************************************************************************************
*   Function: initRoutTab - initialize the the routing table
*************************************************************************************************/
void initRoutTab(Table *tab){
    
    int i=0;

    //number of lines in the table
    tab->rout=(char ***)malloc(MAX_NODES*sizeof(char **));
    if (tab->rout == NULL){
        printf("Memory allocation failure\n");
        exit(1);
    }
    //initialize the first 4 collums that stand for self, sucessor, predecessor, chord
    for(i=0; i<MAX_NODES; i++){
        tab->rout[i]=(char **)malloc(FIX_COLUMN * sizeof(char*));
        if(tab->rout[i]==NULL){
            printf("Memory allocation failure\n");
            exit(1);
        }
        for(int j=0; j<FIX_COLUMN; j++){
            tab->rout[i][j]=strdup("\0");
            if(tab->rout[i][j]==NULL){
                printf("Memory allocation failure\n");
                exit(1);
            }
        }
        tab->map[i]=-1;
    }
     //initialize the vector that helps mapping ids to indices for the lines in the tab
}


/* ***********************************************************************************************
*   Function: freeApp - frees the App
*************************************************************************************************/
void freeRoutTab(NodeApp *app){

    for(int i=0; i<MAX_NODES; i++){
        for(int j=0; j<app->numNo; j++){
            free(app->tab.rout[i][j]);
        }
        free(app->tab.rout[i]);
    }
    free(app->tab.rout);
}

/* ***********************************************************************************************
*   Function: showShortPath - Show me the shortest path
*                           -> command "show path (sp) dest" 
*************************************************************************************************/
void showShortPath(NodeApp *app, char *idDest){
    int i=0;
    char id[3];
    for(i=0; i< MAX_NODES; i++){
        if(app->tab.map[i]!=-1){
            idChar(app->tab.map[i], id);
            if(strcmp(id, idDest)==0){
                printf("\n Node: %s    Shortest path: %s \n", id, app->tab.rout[i][0]);
                return;
            }
        }    
    }
    printf("\n--------Destination not found----------\n");
}


/* ***********************************************************************************************
*   Function: showForwTable - Shows the forwarding table
*                           -> command "show forwarding (sf)" 
*************************************************************************************************/
void showForwTable(NodeApp *app){
    int num_count;
    int *numbers=NULL;
    char id[3], id2[3];
    int i=0;
    
    printf("\n---------Forwarding Table--------\n");
    for(i=0; i <MAX_NODES; i++){
        if(app->tab.map[i]!=-1){
            num_count=0;
            pathVector(app->tab.rout[i][0], &numbers, &num_count);
            idChar(app->tab.map[i],id2);
            if(numbers[0]!=-1){
                idChar(numbers[1],id);
                printf("Node: %s   Neighbour: %s\n", id2, id);
            }else{
                printf("Node: %s   Neighbour: %s\n", id2, id2);
            }
            free(numbers);
        }
    }
    printf("--------------End Table-------------\n");
}

/* ***********************************************************************************************
*   Function: showShortPath - Show me the shortest path
*                           -> command "show routing (sf) dest" 
*************************************************************************************************/
void showRout(NodeApp *app, char *idDest){
    int i=0, index=-1;
    char id[3];
    printf("\n------Routing table of node %s------\n", idDest);
    for(i=0; i< MAX_NODES; i++){
        if(app->tab.map[i]!=-1){
            idChar(app->tab.map[i], id);
            if(strcmp(id,idDest)==0){
                index=i;
                break;
            }
        }    
    }
    for(i=0; i<app->numNo; i++){
        if(app->adj[i].fd!=-1){  
            printf("Neighbour: %s; Path: %s\n", app->adj[i].id, app->tab.rout[index][i]);
        }
    }
}

/* ***********************************************************************************************
*   Function: firstPosFree - return the index of the first free position
*************************************************************************************************/
int firstPosFree(NodeApp *app)
{
    int i=0;
    for(i=0; i<MAX_NODES; i++){
        if(app->tab.map[i]==-1){
            return i;
        }
    }
    return -1;
}


/* ***********************************************************************************************
*   Function: changeColSize - changes the number of columns in the routing table
*************************************************************************************************/
void changeColSize(NodeApp *app, int size){
    int i=0;
    for(i = 0; i <MAX_NODES; i++){
        app->tab.rout[i] = (char**)realloc(app->tab.rout[i], (size) * sizeof(char*));
        if (app->tab.rout[i] == NULL){
            printf("Memory reallocation failed\n");
            exit(1);
        }
    }
}
/* ***********************************************************************************************
*   Function: delNeighb - deletes the information of a neighbouring node
*************************************************************************************************/
void delNeighb(NodeApp *app, int column){
    
    int i=0;
    if(column == app->numNo-1 && column >= FIX_COLUMN){
        for (i = 0; i < MAX_NODES; i++) {
            free(app->tab.rout[i][column]);
        }
        changeColSize(app, app->numNo-1);
        initNode(&(app->adj[column]));

    }else if(column >= FIX_COLUMN){

        //swap information between last column and selected column of routing table and adjacency vector
        for(i=0; i<MAX_NODES; i++){
            free(app->tab.rout[i][column]);
            app->tab.rout[i][column]=strdup(app->tab.rout[i][app->numNo-1]);
            free(app->tab.rout[i][app->numNo-1]);
        }
        changeColSize(app, app->numNo-1);
        updatetNode(&(app->adj[column]), app->adj[app->numNo-1].id, "\0", app->adj[app->numNo-1].fd);   
        initNode(&(app->adj[app->numNo-1]));

    }else{
        //printf("num of columns: %d\n", app->numNo);
        for (i = 0; i < MAX_NODES; i++) {
            free(app->tab.rout[i][column]);
            app->tab.rout[i][column]=strdup("\0");
            
        }
        initNode(&(app->adj[column]));
        return;
    }
    
    app->numNo--;
    printf("num of columns: %d\n", app->numNo);
}


/* ***********************************************************************************************
*   Function: addNeighb - adds a new chord
*************************************************************************************************/
void addChord(NodeApp *app, char *id, int fd){
    
    int i=0;
    updatetNode(&(app->adj[app->numNo]), id, "\0", fd);
    changeColSize(app, app->numNo+1);
    for(i=0; i<MAX_NODES; i++){
        app->tab.rout[i][app->numNo]=strdup("\0");
        if(app->tab.rout[i][app->numNo]==NULL){
            printf("Memory allocation failure\n");
            exit(1);
        }
    }
    app->numNo++;
    return;
}



// /* ********************************************************************************************
// *   Function: distPath - returns the number of times the character "-" appears in a string
// ************************************************************************************************/
int distPath(char *path) {
    int count = 0;
    if(strcmp(path,"-")!=0){
        while (*path) {
            if (*path == '-') {
                count++;
            }
            path++;
        }
    }
    return count+1;
}

/* ********************************************************************************************
*   Function: idChar - adjverts an int into a string 
************************************************************************************************/
void idChar(int num, char *id) {
    id[0] = num / 10 + '0';
    id[1] = num % 10 + '0';
    id[2] = '\0';
}

// /* ********************************************************************************************
// *   Function: pathVector - creates a vector that contains the ids within a string path
// ************************************************************************************************/
void pathVector(char *path, int **numbers, int *num_count) {

    char *aux=strdup(path);

    *num_count = distPath(aux); // Number of dashes
    *numbers = (int *)malloc((*num_count) * sizeof(int));
    if (*numbers == NULL) {
        printf("Memory allocation failed.\n");
    }
    const char *token = strtok(aux, "-");
    int index = 0;
    if(*num_count!=1){
        while (token != NULL) {
            (*numbers)[index++] = atoi(token);
            token = strtok(NULL, "-");
        }
    }else{
        (*numbers)[index++]=-1;
    }
    free(aux);
    return;
}

// char* modifyRouteMessage(const char* message, int ownId, int destinationId) {
//     char* modifiedMessage = (char*)malloc(MAX_MESSAGE_LENGTH * sizeof(char));
//     if (modifiedMessage == NULL) {
//         printf("Memory allocation failed\n");
//         exit(1);
//     }
//     strcpy(modifiedMessage, message);

//     char* token = strtok(modifiedMessage, " ");
//     int neighbourId = atoi(token);
//     token = strtok(NULL, " ");
//     int destId = atoi(token);

//     char* pathToken = strtok(NULL, "-\n"); // Get the path part of the message

//     // Check if ownId is in the path
//     int ownIdInPath = 0;
//     if (pathToken != NULL) {
//         char* pathCopy = strdup(pathToken);
//         char* idToken = strtok(pathCopy, "-");
//         while (idToken != NULL) {
//             if (atoi(idToken) == ownId) {
//                 ownIdInPath = 1;
//                 break;
//             }
//             idToken = strtok(NULL, "-");
//         }
//         free(pathCopy);
//     }

//     // Modify path part of the message
//     if (ownIdInPath) {
//         strcpy(pathToken, "-"); // Replace the path with a hyphen
//     } else {
//         // Shift the existing path to the right by adding ownId to it
//         memmove(pathToken + 3, pathToken, strlen(pathToken) + 1); // Make room for ownId
//         snprintf(pathToken, 3, "%02d", ownId); // Insert ownId
//     }

//     // Create new message
//     char newMessage[MAX_MESSAGE_LENGTH];
//     if (ownIdInPath) {
//         snprintf(newMessage, MAX_MESSAGE_LENGTH, "ROUTE %02d %02d %s", neighbourId, destinationId, pathToken);
//     } else {
//         snprintf(newMessage, MAX_MESSAGE_LENGTH, "ROUTE %02d %02d %02d%s", ownId, destinationId, neighbourId, pathToken);
//     }

//     free(modifiedMessage);
//     return strdup(newMessage);
// }

// /***************************************
//  * Only for testing purposes
//  * 
//  * asfçlasºçlfºçal
// */
void popRope(NodeApp *app){
   
    strcpy(app->adj[3].id, "08");
    app->adj[3].fd=3;
    for(int i=0; i<FIX_COLUMN+2; i++){
        free(app->tab.rout[i][3]);
    }
    app->tab.rout[0][3] = strdup("30-08");
    app->tab.rout[1][3] = strdup("30-08-10");
    app->tab.rout[2][3] = strdup("30-08-12");
    app->tab.rout[3][3] = strdup("-");
    app->tab.rout[4][3] = strdup("30-8-10-21");
    app->tab.rout[5][3] = strdup("-");
 
}

// /***************************************
//  * Only for testing purposes
//  * 
//  * asfçlasºçlfºçal
// */
void popPred(NodeApp *app){

    strcpy(app->adj[2].id, "12");
    app->adj[2].fd= 2;
    for(int i=0; i<FIX_COLUMN+2; i++){
        free(app->tab.rout[i][2]);
    }
    app->tab.rout[0][2]=strdup("30-12-08");
    app->tab.rout[1][2]=strdup("30-12-08-10");
    app->tab.rout[2][2]=strdup("30-12");
    app->tab.rout[3][2]=strdup("-");
    app->tab.rout[4][2]=strdup("30-12-08-10-21");
    app->tab.rout[5][2]=strdup("-");

}
// /***************************************
//  * Only for testing purposes
//  * 
//  * asfçlasºçlfºçal
// */
void popSucc(NodeApp *app){

    for(int i=0; i<FIX_COLUMN+2; i++){
        free(app->tab.rout[i][1]);
    }
    app->adj[1].fd= 1;
    strcpy(app->adj[1].id, "15");
    app->tab.rout[0][1]=strdup("-");
    app->tab.rout[1][1]=strdup("30-15-21-10");
    app->tab.rout[2][1]=strdup("-");
    app->tab.rout[3][1]=strdup("30-15");
    app->tab.rout[4][1]=strdup("30-15-21");
    app->tab.rout[5][1]=strdup("-");

}

void popRopeInt(NodeApp *app){

    int ind=4;
    for(int i=0; i<FIX_COLUMN+2; i++){
        free(app->tab.rout[i][ind]);
    }
    app->adj[ind].fd= 5;
    strcpy(app->adj[ind].id, "25");
    app->tab.rout[0][ind]=strdup("-");
    app->tab.rout[1][ind]=strdup("30-25-21-10");
    app->tab.rout[2][ind]=strdup("-");
    app->tab.rout[3][ind]=strdup("30-25");
    app->tab.rout[4][ind]=strdup("30-25-21");
    app->tab.rout[5][ind]=strdup("-");
    //app->numNo++;

}

void popRopeInt2(NodeApp *app){

    int ind=5;
    for(int i=0; i<FIX_COLUMN+2; i++){
        free(app->tab.rout[i][ind]);
    }
    app->adj[ind].fd= 6;
    strcpy(app->adj[ind].id, "27");
    app->tab.rout[0][ind]=strdup("-");
    app->tab.rout[1][ind]=strdup("30-27-21-10");
    app->tab.rout[2][ind]=strdup("30-27");
    app->tab.rout[3][ind]=strdup("30-27");
    app->tab.rout[4][ind]=strdup("30-27-21");
    app->tab.rout[5][ind]=strdup("-");

}
// /***************************************
//  * Only for testing purposes
//  * 
//  * asfçlasºçlfºçal
// */
void popSelf(NodeApp *app){
    char Ind[6][3] ={"08", "10", "12", "15", "21", "30"};
    app->adj[0].fd= 4;

    int i=0;
    for(i=0; i<6; i++){
        app->tab.map[i]=atoi(Ind[i]);
    }

    for(i=0; i<FIX_COLUMN+2; i++){
        free(app->tab.rout[i][0]);
    }
    strcpy(app->adj[0].id, "30");
    app->tab.rout[0][0]=strdup("30-12-08");
    app->tab.rout[1][0]=strdup("30-12-08-10");
    app->tab.rout[2][0]=strdup("30-12");
    app->tab.rout[3][0]=strdup("30-15");
    app->tab.rout[4][0]=strdup("30-15-21");
    app->tab.rout[5][0]=strdup("30");
}

/* ***********************************************************************************************
*   Function: showShortPath - Show me the shortest path
*************************************************************************************************/
void pShortPath(NodeApp *app){
    int i=0;
    char id[3];
    printf("\n--------Shortest Path Table--------\n");

    for(i=0; i< MAX_NODES; i++){
        if(app->tab.map[i]!=-1){
            idChar(app->tab.map[i], id);
            printf("Neighbour: %s  ", id);
            printf("Path: %s\n", app->tab.rout[i][0]);
        }    
    }
    printf("-------------End Table-------------\n");
}

void pRoutTable(NodeApp *app){
    int i=0, j=0;
    char id[3];

    for(j=0; j<app->numNo; j++){
        if(app->adj[j].fd!=-1){
            printf("\n--------Route Table of Node %s--------\n", app->adj[j].id);
            for(i=0; i< MAX_NODES; i++){
                if(app->tab.map[i]!=-1){
                    idChar(app->tab.map[i], id);
                    printf("Neighbour: %s  ", id);
                    printf("Path: %s\n", app->tab.rout[i][j]);
                }    
            }
        printf("-------------End Table-------------\n");
        }
    }
}

// /***************************************
//  * Only for testing purposes
//  * 
//  * asfçlasºçlfºçal
// */
// void PrintAll(NodeApp app){

//     table *routTab=NULL;
//     printf("\n-------------All Tables------------\n");

//     for(int i=0; i<MAX_NODES; i++){
//         printf("------Table of neighbour: %s-------\n", app.neighbour[i].id);

//         for(routTab=app.neighbour[i].tab; routTab !=NULL; routTab = routTab->next){
//             printf("Node id: %s   Path: %s\n", routTab->id, routTab->path);
//         }
//     }
//     printf("--------------End Table-------------\n");
//     return;
// }


