#include "player.h"
#define PLAYER_NAME "shenzi"

int main(int argc, char **argv) {
    check_args(argc, argv, PLAYER_NAME);
    // fprintf(stderr, "argc: %i, argv0: %s, argv1: %s\n", argc, argv[0], argv[1]);
    play(argv[1]);
}