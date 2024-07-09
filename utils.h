#ifndef UTILS_H_
#define UTILS_H_

#include <stdbool.h>

#define REGIP "193.136.138.142"
#define REGUDP "59000"
// when connecting to node server on LT5, use
// 192.168.1.1 and port 58000
// port might change, prof sena explained this to me.
// go see test platform

void printCommands(void);
int checkArgs(char **argv, int argc);
int validateIPv4(char *addrIP);
int validatePort(char *portNumber);
#endif // !UTILS_H_j
