#ifndef PLAYER_H
#define PLAYER_H

#include "shared.h"

#define NORMAL 0
#define WRONG_ARG_NUM 1
#define INVALID_PLAYER_COUNT 2
#define INVALID_ID 3
#define COMM_ERR 6

enum Argument {
    TOTAL_PLAYERS = 1,
    PLAYER_ID = 2,
};

void exit_with_error(int, char *);
void check_args(int, char **, char *);
void play_game(char *, char *, char *);
void make_move(Game *, Player *, char *);
int process(Game *, Player *, char *);
void process_dowhat(Game *, Player *);
void send_message(char *, ...);
int largest_value(Deck);
Deck get_card_by_value(Deck, int);
int can_afford(Card, Player *);
Deck affordable_cards(Deck, Player *);
void cost_of_card(Card, Player *, int [5]);
int sum_cost(int [5]);
Deck get_card_by_cost(Deck, Player *, int);
int can_take_tokens(Game *, Player *);

#endif