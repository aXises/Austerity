#include "player.h"
#define PLAYER_NAME "shenzi"

int main(int argc, char **argv) {
    check_args(argc, argv, PLAYER_NAME);
    // fprintf(stderr, "argc: %i, argv0: %s, argv1: %s\n", argc, argv[0], argv[1]);
    play_game(argv[1], PLAYER_NAME);
}

void process_dowhat() {
    fprintf(stderr, "---processing dowhat by %s\n", PLAYER_NAME);
    send_message("purchase1:0,0,0,0,0");
    // send_message("take1,1,1,1");
}

void make_move(Game *game, Player *player, char *encoded) {
    process(game, player, encoded);
}