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

void send_message(char *message, ...) {
    va_list args;
    va_start(args, message);
    vprintf(message, args);
    va_end(args);
    fflush(stdout);
}

char *listen(void) {
    char *message = malloc(sizeof(char) * MAX_INPUT);
    if (fgets(message, MAX_INPUT, stdin) == NULL) {
        fprintf(stderr, "MESSAGE FROM HUB IS NULL\n");
    }
    if (message[strlen(message) - 1] != '\0') {
        message[strlen(message) - 1] = '\0';
    }
    return message;
}

int process_tokens(char *encoded) {
    if (strlen(encoded) < 7) {
        return -1;
    }
    char *amount = malloc(0);
    int counter = 0;
    for (int i = 6; i < (strlen(encoded)); i++) {
        if (i != strlen(encoded)) {
            amount = realloc(amount, sizeof(char) * (counter + 1));
        }
        amount[counter] = encoded[i];
        counter++;
    }
    amount = realloc(amount, sizeof(char) * (counter + 1));
    amount[counter] = '\0';
    if (!is_string_digit(amount)) {
        free(amount);
                
        return -1;
    }
    int tokens = atoi(amount);
    free(amount);
    return tokens;
}

void free_game() {
    
}

Player setup_player(char *id, char *name) {
    Player player;
    player.id = atoi(id);
    player.name = name;
    fprintf(stderr, "SETTUNG UP PLAYER %i\n", player.id);
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

int process_newcard(Game *game, char *encoded) {
    char **cardDetails = split(encoded, "d");
    if (strcmp(cardDetails[0], "newcar") != 0 || !check_card(cardDetails[1])) {
        fprintf(stderr, "check card err\n");
        return 0;
    }
    // fprintf(stderr, "str1: %s str2: %s\n", cardDetails[0], cardDetails[1]);
    game->deckFaceup.cards = realloc(game->deckFaceup.cards,
            sizeof(Card) * (game->deckFaceup.amount + 1));
    char **columnSplit = split(cardDetails[1], ":");
    char **commaSplit = split(columnSplit[2], ",");
    Card card;
    card.colour = columnSplit[0][0];
    card.value = atoi(columnSplit[1]);
    for (int i = 0; i < 4; i++) {
        if (strcmp(commaSplit[i], "") == 0) {
            return 0;
        }
        card.cost[i] = atoi(commaSplit[i]);
    }
    game->deckFaceup.cards[game->deckFaceup.amount] = card;
    game->deckFaceup.amount++;
    free(columnSplit);
    free(commaSplit);
    free(cardDetails);
    // fprintf(stderr, "checkcard res: %i\n", check_card(s[1]));
    return 1;
}
    
void process(Game *game, Player *player, char *encoded) {
    if (strstr(encoded, "dowhat") != NULL) {
        process_dowhat(game, player);
        fprintf(stderr, "test---\n");
        send_message("wild\n");
    } else if (strstr(encoded, "tokens") != NULL) {
        //fprintf(stderr, "process %s\n", encoded);
        int tokens = process_tokens(encoded);
        // fprintf(stderr, "processed %i, tokens \n", tokens);
        if (tokens == -1) {
            exit_with_error(COMM_ERR, player->name);
        } else {
            for (int i = 0; i < 4; i++) {
                game->tokenPile[i] = tokens;
            }
        }
        // fprintf(stderr, "player game tokens: %i\n", game->tokenPile[0]);
    } else if (strstr(encoded, "newcard") != NULL) {
        process_newcard(game, encoded);
    } else if (strstr(encoded, "purchased") != NULL) {
        
    } else if (strstr(encoded, "took") != NULL) {
        
    } else if (strstr(encoded, "wild") != NULL) {
        
    } else if (strstr(encoded, "eog") != NULL) {
        
    } else {
        fprintf(stderr, "player protocol err\n");
    }
}

void play_game(char *id, char *name) {
    Game game = setup_game();
    Player player = setup_player(id, name);
    while (1) {
        char *message = listen();
        process(&game, &player, message);
        if (strcmp(message, "eog") == 0) {
            fprintf(stderr, "eog recieved\n");
            free(message);
            break;
        }
        free(message);
    }
    free(player.hand.cards);
    free(game.deckFaceup.cards);
    fprintf(stderr, "player %s shutdown\n", id);
}