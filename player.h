#ifndef PLAYER_H
#define PLAYER_H

#include "shared.h"
#include "util.h"

#define NORMAL 0
#define WRONG_ARG_NUM 1
#define INVALID_PLAYER_COUNT 2
#define INVALID_ID 3
#define COMM_ERR 6

enum argument {
    TOTAL_PLAYERS = 1,
    PLAYER_ID = 2,
};

void exit_with_error(int, char *);
void check_args(int, char **, char*);
void play_game(char *, char *, char *);
void make_move(Game *, Player *, char *);
int process(Game *, Player *, char *);
void process_dowhat(Game *, Player *);
void send_message(char *, ...);

#endif