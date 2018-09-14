#include "player.h"

void exit_with_error(int error) {
    switch(error) {
        case(WRONG_ARG_NUM):
            fprintf(stderr, "Usage: {player} pcount myid\n");
            exit(WRONG_ARG_NUM);
            break;
        case(INVALID_PLAYER_COUNT):
            fprintf(stderr, "Invalid player count\n");
            exit(INVALID_PLAYER_COUNT);
            break;
        case(INVALID_ID):
            fprintf(stderr, "Invalid player ID\n");
            exit(INVALID_ID);
            break;
        case(COMM_ERR):
            fprintf(stderr, "Communication Error\n");
            exit(COMM_ERR);
            break;
        default:
            exit(NORMAL);
            break;
    }
}