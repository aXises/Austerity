#ifndef SHARED_H
#define SHARED_H

#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct Cards {
    char colour;
    int value;
    int discount[4];
} Card;

typedef struct Decks {
    int amount;
    Card *cards;
} Deck;

typedef struct Players {
    char id;
} Player;

#endif