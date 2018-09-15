#include "player.h"

void exit_with_error(int error, char *name) {
    switch(error) {
        case(WRONG_ARG_NUM):
            fprintf(stderr, "Usage: %s pcount myid\n", name);
            break;
        case(INVALID_PLAYER_COUNT):
            fprintf(stderr, "Invalid player count\n");
            break;
        case(INVALID_ID):
            fprintf(stderr, "Invalid player ID\n");
            break;
        case(COMM_ERR):
            fprintf(stderr, "Communication Error\n");
            break;
        default:
            break;
    }
    exit(error);
}

void check_args(int argc, char **argv, char *name) {
    if (argc != 2) {
        exit_with_error(WRONG_ARG_NUM, name);
    }
    if (!is_string_digit(argv[0])) {
        exit_with_error(INVALID_PLAYER_COUNT, name);
    }
    if (!is_string_digit(argv[1])) {
        exit_with_error(INVALID_ID, name);
    }
}

// void send_message(char *message) {
//     printf("");
// }