#ifndef SHARED_H
#define SHARED_H

#include <stdio.h> 
#include <stdlib.h>
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
    char id;
    FILE *input;
    FILE *output;
} Player;

typedef struct {
    int playerAmount;
    Player *players;
    Deck deck;
} Game;

#endif