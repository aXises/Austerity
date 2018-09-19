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
    send_message("purchase1:0,0,0,0,0");
    // send_message("take1,1,1,1");
}

int process_t(char *encoded) {
    if (strlen(encoded) < 7) {
        return -1;
    }
    char *amount = malloc(0);
    int counter = 0;
    for (int i = 6; i < (strlen(encoded)); i++) {
        if (i != strlen(encoded)) {
            char *amount = realloc(amount, sizeof(char) * (i + 1));
        }
        amount[counter] = encoded[i];
        counter++;
    }
    fprintf(stderr, "amount: %s\n", amount);
    free(amount);
    // if (!is_string_digit(amount[1])) {
    //     return -1;
    // }
    return 0;
}

void free_game() {
    
}

Player player_setup() {
    Player player;
    for (int i = 0; i < 4; i++) {
        player.currentDiscount[i] = 0;
        player.tokens[i] = 0;
    }
    player.wildTokens = 0;
    player.hand.amount = 0;
    player.hand.cards = malloc(0);
    return player;
}

Game setup_game() {
    Game game;
    game.deckFaceup.amount = 0;
    game.deckFaceup.cards = malloc(0);
    return game;
}
    
void process(Game *game, char *encoded) {
    if (strstr(encoded, "dowhat") != NULL) {
        // fprintf(stderr, "----process_t: %i\n", process_t("tokens12"));
        process_dowhat();
    } else if (strstr(encoded, "tokens") != NULL) {
        // process_tokens(encoded);
    } else if (strstr(encoded, "newcard") != NULL) {
        
    } else if (strstr(encoded, "purchased") != NULL) {
        
    } else if (strstr(encoded, "took") != NULL) {
        
    } else if (strstr(encoded, "wild") != NULL) {
        
    } else if (strstr(encoded, "eog") != NULL) {
        
    } else {
        fprintf(stderr, "player protocol err\n");
    }
}

void play(char *id) {
    Game game = setup_game();
    //Player player = setup_player();
    while (1) {
        char *message = listen();
        if (strcmp(message, "eog\n") == 0) {
            // fprintf(stderr, "-----%i\n", process_tokens("tokens1\n"));
            free(message);
            break;
        }
        process(&game, message);
        free(message);
    }
    fprintf(stderr, "player %s shutdown\n", id);
}