#include "austerity.h"

void check_args(int argc, char **argv) {
    if (argc < 6 || argc > 26) {
        exit_with_error(WRONG_ARG_NUM);
    }
    for (int i = 1; i < 3; i++) {
        if (!is_string_digit(argv[i])) {
            exit_with_error(BAD_ARG);
        }
    }
}

void exit_with_error(int error) {
    switch(error) {
        case(WRONG_ARG_NUM):
            fprintf(stderr, "Usage: austerity tokens points deck player "\
                    "player [player ...]\n");
            break;
        case(BAD_ARG):
            fprintf(stderr, "Bad argument\n");
            break;
        case(CANNOT_ACCESS_DECK_FILE):
            fprintf(stderr, "Cannot access deck file\n");
            break;
        case(INVALID_DECK_FILE):
            fprintf(stderr, "Invalid deck file contents\n");
            break;
        case(BAD_START):
            fprintf(stderr, "Bad start\n");
            break;
        case(CLIENT_DISCONNECT):
            fprintf(stderr, "Client disconnected\n");
            break;
        case(PROTOCOL_ERR):
            fprintf(stderr, "Protocol error by client\n");
            break;
        case(SIGINT_RECIEVED):
            fprintf(stderr, "SIGINT caught\n");
            break;
    }
    exit(error);
}

void check_card(char *content) {
    if (content[0] != 'B' && content[0] != 'Y' && content[0] != 'P' &&
                content[0] != 'R') {
        exit_with_error(INVALID_DECK_FILE);            
    }
    int colAmount = 0, commaAmount = 0;
    for (int i = 1; i < strlen(content); i++) {
        if (content[i] != ':' && content[i] != ',' && content[i] != '\n'
                && !isdigit(content[i])) {
            exit_with_error(INVALID_DECK_FILE);            
        }
        switch(content[i]) {
            case(':'):
                colAmount++;
                break;
            case(','):
                commaAmount++;
                break;
        }
    }       
    if (colAmount != 2 || commaAmount != 3) {
        exit_with_error(INVALID_DECK_FILE);
    }
}

Deck load_deck(char *fileName) {
    FILE *file = fopen(fileName, "r");
    if (file == NULL || !file) {
        exit_with_error(CANNOT_ACCESS_DECK_FILE);
    }
    char *content = malloc(sizeof(char)), 
    character;
    int counter = 0, cards = 0;
    while((character = getc(file)) != EOF) {
        content = realloc(content, sizeof(char) * (counter + 1));
        content[counter] = character;
        if (character == '\n') {
            cards++;
        }
        counter++;
    }
    fclose(file);
    content = realloc(content, sizeof(char) * (counter + 1));
    content[counter] = '\0';
    char **cardArr = split(content, "\n");
    if (cardArr[0][0]);
    Deck deck;
    deck.amount = cards + 1;
    deck.cards = malloc(sizeof(Card) * deck.amount);
    for (int i = 0; i < deck.amount; i++) {
        check_card(cardArr[i]);
        char **colSplit = split(cardArr[i], ":");
        Card card;
        if (strcmp(colSplit[1], "") == 0) {
            exit_with_error(INVALID_DECK_FILE); 
        }
        card.colour = colSplit[0][0];
        card.value = atoi(colSplit[1]);
        char **commaSplit = split(colSplit[2], ",");
        for (int j = 0; j < 4; j++) {
            if (strcmp(commaSplit[j], "") == 0) {
                exit_with_error(INVALID_DECK_FILE); 
            }
            card.discount[j] = atoi(commaSplit[j]);
        }
        deck.cards[i] = card;
        free(colSplit);
        free(commaSplit);
    }
    free(content);
    free(cardArr);
    return deck;
}

void display_deck(Deck deck) {
    for (int i = 0; i < deck.amount; i++) {
        Card c = deck.cards[i];
        printf("%c %i %i %i %i %i\n", c.colour, c.value, c.discount[0],
                c.discount[1], c.discount[2], c.discount[3]);
    }
}

void free_game(Game game) {
    free(game.deckTotal.cards);
    // free(game.deckFaceup.cards);
    for (int i = 0; i < game.playerAmount; i++) {
        fclose(game.players[i].input);
        fclose(game.players[i].output);
    }
    free(game.players);
    
}

void setup_player(Player *player, char *file, const int playerAmount) {
    int input[2], output[2];
    if (pipe(input) != 0 || pipe(output) != 0) {
        exit_with_error(BAD_START);
    }
    pid_t pid = fork();
    if (pid < 0) {
        exit_with_error(BAD_START);
    } else if (pid > 0) { // Parent Process
        if (close(input[READ]) == -1 || close(output[WRITE]) == -1) {
            exit_with_error(BAD_START);
        }
        player->input = fdopen(input[WRITE], "w");
        player->output = fdopen(output[READ], "r");
        if (player->input == NULL || player->output == NULL) {
            exit_with_error(BAD_START);
        }
    } else { // Child Process
        if (close(input[WRITE]) == -1) {
            exit_with_error(BAD_START);
        }
        if (dup2(input[READ], STDIN_FILENO) == -1) {
            exit_with_error(BAD_START);
        }
        if (close(input[READ]) == -1) {
            exit_with_error(BAD_START);
        }
        if (close(output[READ]) == -1) {
            exit_with_error(BAD_START);
        }
        if (dup2(output[WRITE], STDOUT_FILENO) == -1) {
            exit_with_error(BAD_START);
        }
        if (close(output[WRITE]) == -1) {
            exit_with_error(BAD_START);
        }
        player->pid = pid;
        char playerAmountArg[2];
        playerAmountArg[0] = playerAmount + '0';
        playerAmountArg[1] = '\0';
        char playerIdArg[2];
        playerIdArg[0] = (player->id) + '0';
        playerIdArg[1] = '\0';
        execlp(file, playerAmountArg, playerIdArg, NULL);
        exit_with_error(BAD_START);
    }
}

Player *setup_players(char **playerPaths, const int amount) {
    Player *players = malloc(0);
    for (int i = 0; i < amount; i++) {
        if (i != amount) {
            players = realloc(players, sizeof(Player) * (i + 1));
        }
        Player player;
        player.id = i;
        setup_player(&player, playerPaths[i], amount);
        players[i] = player;
    }
    return players;
}

void send_message(Player player, char *message) {
    fprintf(player.input, "%s\n", message);
    fflush(player.input);
}

void buy_card(Game *game, Player *player, int index) {
    
}

void draw_cards(Game *game) {
    if (game->deckTotal.amount < 8) {
        game->deckFaceup.cards = game->deckTotal.cards;
        game->deckFaceup.amount = game->deckTotal.amount;
        game->deckIndex = game->deckTotal.amount;
    } else {
        game->deckFaceup.cards = malloc(0);
        for (int i = game->deckIndex; i < game->deckTotal.amount; i++) {
            if (i != game->deckTotal.amount) {
                game->deckFaceup.cards = realloc(game->deckFaceup.cards,
                        sizeof(Card) * (i + 1));
            }
            game->deckFaceup.cards[i] = game->deckTotal.cards[i];
            game->deckIndex++;
            if (i == 8) {
                break;
            }
        }
        game->deckFaceup.amount = 8;
    }
}

char *listen(Player player) {
    char *message = malloc(sizeof(char) * MAX_INPUT);
    //size_t length = (size_t) MAX_INPUT;
    if (fgets(message, MAX_INPUT, player.output) == NULL) {
        printf("hub err\n");
    }
    printf("recieved from player: %s", message);
    return message;
}

int match_seperators(char *str, int expectedColumn, int expectedComma) {
    int colAmount = 0, commaAmount = 0;
    for (int i = 0; i < (strlen(str) - 1); i++) {
        switch(str[i]) {
            case(':'):
                colAmount++;
                break;
            case(','):
                commaAmount++;
                break;
        }
    }
    return colAmount == expectedColumn && commaAmount == expectedComma; 
}

void process_purchase(Game *game, Player *player, char *encoded) {
    printf("**processing purchase\n");
    if (!match_seperators(encoded, 1, 4)) {
        printf("purchase com err\n");
    }
    char **colSplit = split(encoded, ":");
    if (strlen(colSplit[0]) != 9 || strlen(colSplit[1]) != 10) {
        printf("purchase com err\n");
    }
    printf("*** %s -- %s\n", colSplit[0], colSplit[1]);
    for (int i = 0; i < 9; i++) {
        if (colSplit[1][i] != ',' && colSplit[1][i] != '\n'
                && !isdigit(colSplit[1][i])) {
            printf("purchase com err\n");  
        }
    }
    free(colSplit);
    
}


void process(Game *game, Player *player, char *encoded) {
    printf("processing message from: %c\n", player->id + 'A');
    if (strstr(encoded, "purchase") != NULL) {
        process_purchase(game, player, encoded);
    } else if (strstr(encoded, "take") != NULL) {
        printf("**processing take\n");   
    } else if (strstr(encoded, "wild") != NULL) {
        printf("**processing wild\n");   
    } else {
        printf("protocol err\n");
    }
}

void init_game(Game *game) {
    game->deckIndex = 0;
    game->deckFaceup.amount = 0;
}

void play_game(Game *game) {
    init_game(game);
    draw_cards(game);
    printf("faceup: %i total: %i\n", game->deckFaceup.amount, game->deckTotal.amount);
    display_deck(game->deckFaceup);
    printf("Comm started -----\n");
    for (int i = 0; i < game->playerAmount; i++) {
        send_message(game->players[i], "test");
        char *message = listen(game->players[i]);
        process(game, &(game->players[i]), message);
        free(message);
        send_message(game->players[i], "exit");
    }
}

int main(int argc, char **argv) {
    check_args(argc, argv);
    Game game;
    game.deckTotal = load_deck(argv[3]);
    game.winPoints = atoi(argv[2]);
    // 
    char **playersPaths = malloc(0);
    game.playerAmount = 0;
    for (int i = 4; i < argc; i++) {
        if (i != argc) {
            playersPaths = realloc(playersPaths, sizeof(char *) * (i - 3));
        }
        playersPaths[i - 4] = argv[i];
        game.playerAmount++;
    }
    for (int i = 0; i < 4; i++) {
        game.tokenPile[i] = atoi(argv[1]);
    }
    game.players = setup_players(playersPaths, game.playerAmount);
    play_game(&game);
    // printf("%i\n", game.playerAmount);
    free(playersPaths);
    free_game(game);
}
