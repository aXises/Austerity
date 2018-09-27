#ifndef SHARED_H
#define SHARED_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

/** 
 * Constants read, write and max inputs.
 */
#define READ 0
#define WRITE 1
#define MAX_INPUT 256

/** 
 * Constants for true and false boolean values.
 */
#define TRUE 1
#define FALSE 0

/** 
 * Enum for indexes of tokens. 
 */
enum Tokens {
    PURPLE = 0,
    BROWN = 1,
    YELLOW = 2,
    RED = 3,
    WILD = 4
};

/** 
 * Type defination for an card.
 */
typedef struct {
    char colour; // Discount colour.
    int value; // Value of the card.
    int cost[4]; // Token cost in order P, B, Y, R
} Card;

/** 
 * Type defination for an deck.
 */
typedef struct {
    int amount; // Total amount of cards in this deck.
    Card *cards; // An array of cards.
} Deck;

/** 
 * Type defination for an player.
 */
typedef struct {
    char *name; // The name of the player.
    pid_t pid; // The pid of the player.
    int id; // The player id.
    FILE *input; // Input stream for the player.
    FILE *output; // output stream.
    int currentDiscount[4]; // Discounts the player has in order P, B, Y, R
    int tokens[4]; // Tokens the player has in order P, B, Y, R
    int wildTokens; // Amount of wild tokens the player owns.
    int points; // Amount of points the player has.
} Player;

/** 
 * Type defination for an Game.
 */
typedef struct {
    int playerAmount; // Total amount of players.
    Player *players; // An array of players.
    Deck deckTotal; // The deck of all cards loaded.
    Deck deckFaceup; // The deck of cards which are facing up.
    int deckIndex; // The current index of the total deck.
    int tokenPile[4]; // Token pile the player can take from.
    int winPoints; // Amount of points required to win the game.
} Game;

/**
 * Function Prototypes
 **/
int index_of_card(Deck, Card, int);
void set_player_values(Player *);
int is_string_digit(char *);
char **split(char *, char *);
void display_deck(Deck);
int check_encoded(char **, int);
int check_card(char *);
int match_seperators(char *, const int, const int);
void update_discount(char, Player *);
int get_highest_points(Game);
void get_winners(Game *, int, int);

#endif