#ifndef SHARED_H
#define SHARED_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#define READ 0
#define WRITE 1

typedef struct {
    char colour;
    int value;
    int discount[4];
} Card;

typedef struct {
    int amount;
    Card *cards;
} Deck;

typedef struct {
    pid_t pid;
    int id;
    FILE *input;
    FILE *output;
} Player;

typedef struct {
    int playerAmount;
    Player *players;
    Deck deck;
} Game;

int is_string_digit(char *);
char **split(char *, char *);

#endif