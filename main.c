#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include "utils.h"
#include "topologyP.h"
#include "userInterface.h"

int main(int argc, char *argv[]) {
	int maxfd = 0, activity = 0, newfd = 0, n = 0, i = 0;
	int iterations = 0, tmp = 0;
	fd_set readfds;
	char buffer[MAX_BUFFER_SIZE];
	socklen_t addrlen;
	struct sockaddr_in addr;
	addrlen = sizeof(struct sockaddr_in);
	NodeApp app;

	n = checkArgs(argv, argc);
	if(n == 1) {
		strcpy(app.ipUDP, REGIP);
		strcpy(app.portUDP, REGUDP);
	} else {
		strcpy(app.ipUDP, argv[3]);
		strcpy(app.portUDP, argv[4]);
	}

	initNodeApp(&app, argv[1], argv[2]);
	printCommands();

	while (1) {
		// Clear the set of file descriptors
		maxfd = 0;
		FD_ZERO(&readfds);
		// Add stdin and self to the set
		FD_SET(STDIN_FILENO, &readfds);

		//Checks if the node belongs to a ring
		if(app.ring[0] != '\0'){

			FD_SET(app.adj[0].fd, &readfds); //trigger to the listening socket

			//trigger to the predecessor socket
			if(app.adj[2].fd != -1) {
				FD_SET(app.adj[2].fd, &readfds);
				maxfd = maxfd > app.adj[2].fd ? maxfd : app.adj[2].fd;
			}
			//trigger to the sucessor socket
			if(app.adj[1].fd!=-1) {
				FD_SET(app.adj[1].fd, &readfds);
				maxfd = maxfd > app.adj[1].fd ? maxfd : app.adj[1].fd;
			}
			maxfd = maxfd > app.adj[0].fd ? maxfd : app.adj[0].fd;
			if(app.tempfd != -1) {
				FD_SET(app.tempfd, &readfds);
				maxfd = maxfd > app.tempfd ? maxfd : app.tempfd;
			}
			for(i = 3; i < app.numNo; i++) {
				if(app.adj[i].fd!=-1) {
					FD_SET(app.adj[i].fd, &readfds);
					maxfd = maxfd > app.adj[i].fd ? maxfd : app.adj[i].fd;
				}
			}
		} else {
			maxfd = STDIN_FILENO;
		}


		// Use select to wait for activity on any of the file descriptors
		activity = select(maxfd + 1, &readfds, NULL, NULL, NULL);
		if (activity == -1) {
			perror("Error in select");
			closeProgram(&app);
		}
		iterations = 0;

		while(activity != 0) {
			tmp = activity;
			// Check if there is data to read from stdin
			if (FD_ISSET(STDIN_FILENO, &readfds)){
				FD_CLR(STDIN_FILENO, &readfds);
				activity--; 


				fgets(buffer, MAX_READ, stdin);
				if(!strncmp(buffer, "UNREG ", 6)){
					UdpSocket(&app, buffer);
					fprintf(stderr, "%s\n", app.buffer);

				} else if(!strncmp(buffer, "NODES ", 6)){
					UdpSocket(&app, buffer);
					fprintf(stderr, "%s\n", app.buffer);
				} else{
					readStdin(&app, buffer);
					memset(buffer, 0, MAX_BUFFER_SIZE);
				}
			}

			// Check if there are new TCP connections in listening socket
			if (app.adj[0].fd != -1) {
				if(FD_ISSET(app.adj[0].fd, &readfds)){
					FD_CLR(app.adj[0].fd, &readfds);
					activity--; 

					newfd = accept(app.adj[0].fd, (struct sockaddr *)&addr, &addrlen);
					/**/
					n = read(newfd, app.adj[0].buffer, MAX_READ);
					if(n == -1) {
						fprintf(stderr, "Error: Reading from a client\n");
						closeProgram(&app);
					}
					buffer[n] = '\0';
					readClient(&app, &app.adj[0], newfd);
				}
			}


			// checks if there are messages from SUCCESSOR
			if(app.adj[1].fd!=-1) {
				// read message from SUCCESSOR
				if(FD_ISSET(app.adj[1].fd, &readfds)){
					FD_CLR(app.adj[1].fd, &readfds);
					activity--; 

					n = read(app.adj[1].fd, app.adj[1].buffer, MAX_READ);
					if (n == -1) {
						fprintf(stderr, "Error reading from successor\n");
						closeProgram(&app);
					} else if(n == 0) {
						handleSucLeave(&app, app.adj[1].fd);
					} else {
						app.adj[1].buffer[n] = '\0';
						readServer(&app, &app.adj[1]);
					}
				}
			}

			// Checks if there are messages from PREDECESSOR
			if(app.adj[2].fd!=-1)
			{
				if(FD_ISSET(app.adj[2].fd, &readfds))
				{
					FD_CLR(app.adj[2].fd, &readfds);
					activity--; 

					/**/
					n = read(app.adj[2].fd, app.adj[2].buffer, MAX_READ);
					if (n == -1) {
						fprintf(stderr, "Error reading from my pred\n");
						closeProgram(&app);
					} else if(n == 0){
						close(app.adj[2].fd);
						initNode(&app.adj[2]);
					} else {
						app.adj[2].buffer[n] = '\0';
						readClient(&app, &app.adj[2], app.adj[2].fd);
					}
				}
			}

			// read message from CHORD
			if(app.adj[3].fd != -1) {
				if(FD_ISSET(app.adj[3].fd, &readfds)){
					FD_CLR(app.adj[3].fd, &readfds);
					activity--; 

					n = read(app.adj[3].fd, app.adj[3].buffer, MAX_READ);
					if (n == -1) {
						fprintf(stderr, "Error reading from successor\n");
						closeProgram(&app);
					} else if(n == 0) {
						close(app.adj[3].fd);
						initNode(&app.adj[3]);
					} else {
						app.adj[3].buffer[n] = '\0';
						readServer(&app, &app.adj[3]);
					}
				}
			}

			for (i = 4; i < app.numNo; i++) {
				if(app.adj[i].fd != -1) {
					if(FD_ISSET(app.adj[i].fd, &readfds)) {
						FD_CLR(app.adj[i].fd, &readfds);
						activity--; 

						/**/
						n = read(app.adj[i].fd, app.adj[i].buffer, MAX_READ);
						if (n == -1) {
							fprintf(stderr, "Error reading from my pred\n");
							closeProgram(&app);
						} else if(n == 0) {
							close(app.adj[i].fd);
							initNode(&app.adj[i]);
						} else {
							app.adj[i].buffer[n] = '\0';
							readClient(&app, &app.adj[i], app.adj[i].fd);
						}
					}
				}
			}

			// Checks if there are messages from PREDECESSOR
			if(app.tempfd != -1)
			{
				if(FD_ISSET(app.tempfd, &readfds))
				{
					FD_CLR(app.tempfd, &readfds);
					activity--; 

					n = read(app.tempfd, buffer, MAX_READ);
					if (n == -1) {
						fprintf(stderr, "Error reading from tempfd\n");
						closeProgram(&app);
					} else if(n == 0) {
						close(app.tempfd);
						app.tempfd = -1;
					} else {
						buffer[n] = '\0';
					}
				}
			}
			if(tmp == activity) {
				iterations++;
			}
			if(iterations > 3) {
				activity = 0;
			}
		}
	}

	return EXIT_SUCCESS;
}
