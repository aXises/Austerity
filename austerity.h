#ifndef AUSTERITY_H
#define AUSTERITY_H

#include <stdio.h> 
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#define NORMAL 0
#define WRONG_ARG_NUM 1
#define BAD_ARG 2
#define CANNOT_ACCESS_DECK_FILE 3
#define INVALID_DECK_FILE 4
#define BAD_START 5
#define CLIENT_DISCONNECT 6
#define PROTOCOL_ERR 7
#define SIGINT_RECIEVED 10

void checkError(int error);

#endif