#ifndef AUSTERITY_H
#define AUSTERITY_H

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include "shared.h"

/** 
 * Constants for hub error codes.
 */
#define NORMAL 0
#define WRONG_ARG_NUM 1
#define BAD_ARG 2
#define CANNOT_ACCESS_DECK_FILE 3
#define INVALID_DECK_FILE 4
#define BAD_START 5
#define CLIENT_DISCONNECT 6
#define PROTOCOL_ERR 7
#define SIGINT_RECIEVED 10

/** 
 * Enum for indexes of argv. 
 */
enum Argument {
    TOKENS = 1,
    WIN_POINTS = 2,
    DECK_FILE = 3
};

/**
 * Function Prototypes
 **/
void free_game(Game *game);
void kill_children(Game *game);
void wait_children(Game *game);
void check_args(int argc, char **argv);
void exit_with_error(Game *game, int error);
void parse_deck(Deck deck, char **cardArr);
Deck load_deck(char *fileName);
void setup_parent(Game *game, int id, int input[2],
        int output[2], int test[2]);
void setup_child(Game *game, int id, int input[2], int output[2],
        int test[2], char *file);
void setup_player(Game *game, int id, char *file);
void setup_players(Game *game, char **playerPaths);
void send_message(Player player, char *message, ...);
void send_all(Game *game, char *message, ...);
int has_next_card(Game *game);
void display_card(Card c);
void draw_next(Game *game, Deck *deck);
void draw_cards(Game *game);
void buy_card(Game *game, Player *player, int index);
char *listen(Player player);
int use_tokens(Game *game, Player *player, Card card, int tokens[5]);
int process_purchase(Game *game, Player *player, char *encoded);
int check_take(Game *game, char *content);
int process_take(Game *game, Player *player, char *encoded);
int process_wild(Game *game, Player *player, char *encoded);
int process(Game *game, Player *player, char *encoded);
void init_game(Game *game);
int game_is_over(Game game);
void play_game(Game *game);
void setup_signal_action(void);
void sigint_handle(int sig);

#endif