#ifndef SHARED_H
#define SHARED_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#define READ 0
#define WRITE 1
#define MAX_INPUT 256

enum Tokens {
    PURPLE = 0,
    BLUE = 1,
    YELLOW = 2,
    RED = 3
};

typedef struct {
    char colour;
    int value;
    int cost[4]; // P, B, Y, R
} Card;

typedef struct {
    int amount;
    Card *cards;
} Deck;

typedef struct {
    char *name;
    pid_t pid;
    int id;
    FILE *input;
    FILE *output;
    int currentDiscount[4]; // P, B, Y, R
    int tokens[4]; // P, B, Y, R
    int wildTokens;
    int points;
    Deck hand;
} Player;

typedef struct {
    int playerAmount;
    Player *players;
    Deck deckTotal;
    Deck deckFaceup;
    int deckIndex;
    int tokenPile[4]; // P, B, Y, R
    int winPoints;
} Game;


int is_string_digit(char *);
char **split(char *, char *);
void display_deck(Deck);
int check_card(char *);
int match_seperators(char *, int , int);

#endif