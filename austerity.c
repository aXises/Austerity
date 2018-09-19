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

void check_card(char *content) {
    if (content[0] != 'B' && content[0] != 'Y' && content[0] != 'P' &&
                content[0] != 'R') {
        exit_with_error(INVALID_DECK_FILE);            
    }
    if (!match_seperators(content, 2, 3)) {
        exit_with_error(INVALID_DECK_FILE);
    }
    for (int i = 1; i < strlen(content); i++) {
        if (content[i] != ':' && content[i] != ',' && content[i] != '\n'
                && !isdigit(content[i])) {
            exit_with_error(INVALID_DECK_FILE);            
        }
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
            card.cost[j] = atoi(commaSplit[j]);
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
        printf("%i: %c %i %i %i %i %i\n", i, c.colour, c.value, c.cost[0],
                c.cost[1], c.cost[2], c.cost[3]);
    }
}

void free_game(Game game) {
    if (game.deckFaceup.amount > 0) {
        free(game.deckTotal.cards);
    }
    free(game.deckFaceup.cards);
    for (int i = 0; i < game.playerAmount; i++) {
        fclose(game.players[i].input);
        fclose(game.players[i].output);
        if (game.players[i].hand.amount > 0)
            free(game.players[i].hand.cards);
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
        player.wildTokens = 0;
        player.hand.amount = 0;
        player.points = 0;
        player.hand.cards = malloc(0);
        for (int j = 0; j < 4; j++) {
            player.currentDiscount[j] = 0;
            player.tokens[j] = 0;
        }
        setup_player(&player, playerPaths[i], amount);
        players[i] = player;
    }
    return players;
}

void send_message(Player player, char *message) {
    fprintf(player.input, "%s\n", message);
    fflush(player.input);
}

int has_next_card(Game *game) {
    return game->deckTotal.amount != game->deckIndex;
}

void draw_next(Game *game, Deck *deck) {
    if (has_next_card(game)) {
        Card card = game->deckTotal.cards[(game->deckIndex) - 1];
        game->deckIndex++;
        deck->cards[7] = card;
        deck->amount++;
    }
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
    if (fgets(message, MAX_INPUT, player.output) == NULL) {
        printf("hub err\n");
    }
    printf("recieved from player: %s", message);
    return message;
}

void buy_card(Game *game, Player *player, int index) {
    Deck newDeck;
    newDeck.cards = malloc(0);
    newDeck.amount = 0;
    player->hand.cards = realloc(player->hand.cards, player->hand.amount + 1);
    printf("==hand: %i\n", player->hand.amount);
    player->hand.cards[player->hand.amount] = game->deckFaceup.cards[index];
    player->hand.amount++;
    for (int i = 0; i < (game->deckFaceup.amount - 1); i++) {
        newDeck.cards = realloc(newDeck.cards, sizeof(Card) * (i + 1));
        if (i < index) {
            newDeck.cards[i] = game->deckFaceup.cards[i];
        } else {
            newDeck.cards[i] = game->deckFaceup.cards[i + 1];
        }
        newDeck.amount++;
    }
    if (!has_next_card(game)) {
        game->deckFaceup.amount--;
    }
    newDeck.cards = realloc(newDeck.cards,
            sizeof(Card) * (newDeck.amount + 1));
    free(game->deckFaceup.cards);
    draw_next(game, &newDeck);
    game->deckFaceup.cards = newDeck.cards;
}

int use_tokens(Player *player, Card card, int tokens[4], int wild) {
    printf("Player %c purchased\n", player->id + 'A');
    for (int i = 0; i < 4; i++) {
        if (player->tokens[i] < tokens[i]) {
            return 0;
        }
    }
    int usedWild = 0, acutalPrice;
    for (int i = 0; i < 4; i++) {
        acutalPrice = card.cost[i] - player->currentDiscount[i];
        if (tokens[i] < acutalPrice) {
            int difference = acutalPrice - tokens[i];
            if (difference > (wild - usedWild)) {
                printf("not enough %i\n", i);
                return 0;
            } else {
                printf("%i used, using %i on %i\n", usedWild, difference, i);
                usedWild += difference;
            }
        }
    }
    for (int i = 0; i < 4; i++) {
        player->tokens[i] -= tokens[i];
    }
    player->wildTokens -= wild;
    printf("token: %i\n", wild);
    return 1;
}

int process_purchase(Game *game, Player *player, char *encoded) {
    if (!match_seperators(encoded, 1, 4)) {
        return 0;
    }
    char **colSplit = split(encoded, ":");
    if (strlen(colSplit[0]) != 9 || strlen(colSplit[1]) != 10) {
        return 0;
    }
    printf("*** %s -- %s\n", colSplit[0], colSplit[1]);
    for (int i = 0; i < 9; i++) {
        if (colSplit[1][i] != ',' && colSplit[1][i] != '\n'
                && !isdigit(colSplit[1][i])) {
            return 0;
        }
    }
    if (atoi(colSplit[0]) > 7) {
        return 0;
    }
    int index = colSplit[0][strlen(colSplit[0]) - 1] - '0';
    int tokens[4];
    char **commaSplit = split(colSplit[1], ",");
    for (int i = 0; i < 4; i++) {
        tokens[i] = atoi(commaSplit[i]);
    }
    Card card = game->deckFaceup.cards[index];
    int status = use_tokens(player, card, tokens, colSplit[1][8]  - '0');
    if (!status) {
        return 0;
    }
    switch (card.colour) {
        case('P'):
            player->currentDiscount[0]++;
            break;
        case('B'):
            player->currentDiscount[1]++;
            break;
        case('Y'):
            player->currentDiscount[2]++;
            break;
        case('R'):
            player->currentDiscount[3]++;
            break;
    }
    player->points += card.value;
    buy_card(game, player, 1);
    free(colSplit);
    free(commaSplit);
    return 1;
}

int process_take(Game *game, Player *player, char *encoded) {
    if (!match_seperators(encoded, 0, 3)) {
        return 0;
    }
    char **commaSplit = split(encoded, ",");
    if (strlen(commaSplit[0]) != 5) {
        return 0;
    }
    for (int i = 4; i < strlen(encoded); i++) {
        if (encoded[i] != ',' && encoded[i] != '\n'
                && !isdigit(encoded[i])) {
            return 0;
        }
    }
    for (int i = 0; i < 4; i++) {
        if (i == 0) {
            printf("encoded: %c\n", commaSplit[0][4]);
            if ((commaSplit[0][4] - '0') > game->tokenPile[0]) {
                return 0;
            }
        } else {
            printf("encoded: %c\n", commaSplit[i][0]);
            if ((commaSplit[i][0] - '0') > game->tokenPile[i]) {
                return 0;
            }
        }
    }
    for (int i = 0; i < 4; i++) {
        if (i == 0) {
            game->tokenPile[0] -= (commaSplit[0][4] - '0');
            player->tokens[0] += (commaSplit[0][4] - '0');
        } else {
            game->tokenPile[i] -= (commaSplit[i][0] - '0');
            player->tokens[i] += (commaSplit[i][0] - '0');
        }
    }
    free(commaSplit);
    return 1;
}

int process(Game *game, Player *player, char *encoded) {
    // printf("processing message from: %c\n", player->id + 'A');
    if (strstr(encoded, "purchase") != NULL) {
        return process_purchase(game, player, encoded);
    } else if (strstr(encoded, "take") != NULL) {
        printf("**processing take\n");
        return process_take(game, player, encoded);
    } else if (strstr(encoded, "wild") != NULL) {
        return 1;
        printf("**processing wild\n");   
    } else {
        return 0;
        printf("protocol err\n");
    }
}

void init_game(Game *game) {
    game->deckIndex = 0;
    game->deckFaceup.amount = 0;
}

void send_all(Game *game, char *message) {
    for (int i = 0; i < game->playerAmount; i++) {
        send_message(game->players[i], message);
    }
}

void play_game(Game *game) {
    init_game(game);
    draw_cards(game);
    printf("faceup: %i total: %i\n", game->deckFaceup.amount, game->deckTotal.amount);
    display_deck(game->deckFaceup);
    printf("Comm started -----\n");
    int gameOver = 0;
    while (!gameOver) {
        send_all(game, "dowhat");
        for (int i = 0; i < game->playerAmount; i++) {
            char *message = listen(game->players[i]);
            if (!process(game, &(game->players[i]), message)) {
                printf("protocol err 1st\n");
                send_message(game->players[i], "dowhat");
                char *repromptMessage = listen(game->players[i]);
                if (!process(game, &(game->players[i]), repromptMessage)) {
                    printf("protocol err terminating\n");
                    send_all(game, "eog");
                    exit_with_error(PROTOCOL_ERR);
                }
            }
            free(message);
            printf("Discounts:");
            for (int j = 0; j < 4; j++) {
                printf("%i,", game->players[i].currentDiscount[j]);
            }
            printf("\n");
        }
        gameOver = 1;
    }
    send_all(game, "eog");
}

int main(int argc, char **argv) {
    check_args(argc, argv);
    Game game;
    game.deckTotal = load_deck(argv[3]);
    game.winPoints = atoi(argv[2]);
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
    free(playersPaths);
    free_game(game);
}
