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
    if (fgets(message, MAX_INPUT, stdin) == NULL) {
        fprintf(stderr, "player comm err\n");
    }
    return message;
}

void process_dowhat() {
    fprintf(stderr, "---processing dowhat\n");
    // send_message("purchase1:2,3,0,2,0");
    send_message("take1,1,1,1");
}

void process(char *encoded) {
    if (strstr(encoded, "dowhat") != NULL) {
        process_dowhat();
    } else if (strstr(encoded, "tokens") != NULL) {
        
    } else if (strstr(encoded, "newcard") != NULL) {
        
    } else if (strstr(encoded, "purchased") != NULL) {
        
    } else if (strstr(encoded, "took") != NULL) {
        
    } else if (strstr(encoded, "wild") != NULL) {
        
    } else if (strstr(encoded, "eog") != NULL) {
        
    } else {
        fprintf(stderr, "player protocol err\n");
    }
}

void play(char *name) {
    while (1) {
        char *message = listen();
        if (strcmp(message, "eog\n") == 0) {
            free(message);
            break;
        }
        process(message);
        free(message);
    }
    fprintf(stderr, "player %s shutdown\n", name);
}