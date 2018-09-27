#ifndef PLAYER_H
#define PLAYER_H

#include "shared.h"

/** 
 * Constants for player error codes.
 */
#define NORMAL 0
#define WRONG_ARG_NUM 1
#define INVALID_PLAYER_COUNT 2
#define INVALID_ID 3
#define COMM_ERR 6

/** 
 * Enum for indexes of argv. 
 */
enum Argument {
    TOTAL_PLAYERS = 1,
    PLAYER_ID = 2,
};

/**
 * Function Prototypes
 **/
void process_dowhat(Game *game, Player *player);
void exit_with_error(int error, char *name);
void check_args(int argc, char **argv, char *name);
void send_message(char *message, ...);
char *listen(void);
int process_tokens(char *encoded);
void free_game(Game *game);
Player setup_player(int id, char *name);
Game setup_game(int amount);
void remove_card(Deck *deck, int index);
void add_card(Deck *deck, Card card);
int process_newcard(Game *game, char *encoded);
int process_took(Game *game, char *encoded);
int process_wild(Game *game, char *encoded);
void update_tokens(Game *game, Player *player, int tokens[5]);
int process_purchase(Game *game, char *encoded);
void display_stats(Game *game);
int process(Game *game, Player *player, char *encoded);
Player *setup_players(const int amount);
void play_game(char *amount, char *id, char *name);
int largest_value(Deck deck);
Deck get_card_by_value(Deck deck, int value);
int can_afford(Card card, Player *player);
Deck affordable_cards(Deck deck, Player *player);
void cost_of_card(Card card, Player *player, int finalCost[5]);
int sum_cost(int cost[5]);
Deck get_card_by_cost(Deck deck, Player *player, int costTotal);
int can_take_tokens(Game *game, Player *player);
void purchase_card(Game *game, Player *player, int index);

#endif