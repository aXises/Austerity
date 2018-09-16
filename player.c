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

void send_message(char *message) {
    printf("%s\n", message);
    fflush(stdout);
}

char *listen(void) {
    char *message = malloc(sizeof(char) * MAX_INPUT);
    //size_t length = (size_t) MAX_INPUT;
    if (fgets(message, MAX_INPUT, stdin) == NULL) {
        fprintf(stderr, "comm err\n");
    }
    fprintf(stderr, "recieved from hub: %s", message);
    return message;
}

void play(char *name) {
    while (1) {
        char *message = listen();
        send_message("purchase1:1,1,1,1,0");
        if (strcmp(listen(), "exit\n") == 0) {
            free(message);
            break;
        }
    }
    fprintf(stderr, "ended\n");
}