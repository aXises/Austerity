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
        case (WRONG_ARG_NUM):
            fprintf(stderr, "Usage: austerity tokens points deck player "\
                    "player [player ...]\n");
            break;
        case (BAD_ARG):
            fprintf(stderr, "Bad argument\n");
            break;
        case (CANNOT_ACCESS_DECK_FILE):
            fprintf(stderr, "Cannot access deck file\n");
            break;
        case (INVALID_DECK_FILE):
            fprintf(stderr, "Invalid deck file contents\n");
            break;
        case (BAD_START):
            fprintf(stderr, "Bad start\n");
            break;
        case (CLIENT_DISCONNECT):
            fprintf(stderr, "Client disconnected\n");
            break;
        case (PROTOCOL_ERR):
            fprintf(stderr, "Protocol error by client\n");
            break;
        case (SIGINT_RECIEVED):
            fprintf(stderr, "SIGINT caught\n");
            break;
    }
    exit(error);
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
        if (!check_card(cardArr[i])) {
            exit_with_error(INVALID_DECK_FILE);
        }
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

void free_game(Game game) {
    if (game.deckTotal.amount < 8) {
        free(game.deckTotal.cards);
    } else {
        if (game.deckTotal.amount > 0) {
            free(game.deckTotal.cards);
        }
        if (game.deckFaceup.amount >= 0) {
            free(game.deckFaceup.cards);
        }
    }
    for (int i = 0; i < game.playerAmount; i++) {
        fclose(game.players[i].input);
        fclose(game.players[i].output);
        // free(game.players[i].hand.cards);
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
        int devNull = open("/dev/null", O_WRONLY);
        if (devNull == -1) {
            exit_with_error(BAD_START);
        }
        if (dup2(devNull, STDERR_FILENO) == -1) {
            exit_with_error(BAD_START);
        }
        player->pid = pid;
        char playerAmountArg[3], playerIdArg[3];
        playerAmountArg[2] = '\0', playerIdArg[2] = '\0';
        sprintf(playerAmountArg, "%d", playerAmount);
        sprintf(playerIdArg, "%d", player->id);
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
        set_player_values(&player);
        setup_player(&player, playerPaths[i], amount);
        players[i] = player;
    }
    return players;
}

void send_message(Player player, char *message, ...) {
    va_list args;
    va_start(args, message);
    vfprintf(player.input, message, args);
    va_end(args);
    fflush(player.input);
}

void send_all(Game *game, char *message, ...) {
    for (int i = 0; i < game->playerAmount; i++) {
        va_list args;
        va_start(args, message);
        vfprintf(game->players[i].input, message, args);
        fflush(game->players[i].input);
        va_end(args);
    }
}

int has_next_card(Game *game) {
    return game->deckTotal.amount != game->deckIndex;
}

void display_card(Card c) {
    printf("New Card = Bonus %c, worth %i, costs %i,%i,%i,%i\n", c.colour, 
            c.value, c.cost[PURPLE], c.cost[BLUE],
            c.cost[YELLOW], c.cost[RED]);
}

void draw_next(Game *game, Deck *deck) {
    if (has_next_card(game)) {
        Card card = game->deckTotal.cards[(game->deckIndex) - 1];
        game->deckIndex++;
        deck->cards[7] = card;
        deck->amount++;
        display_card(card);
        send_all(game, "newcard%c:%i:%i,%i,%i,%i\n", card.colour, card.value,
                card.cost[PURPLE], card.cost[BLUE],
                card.cost[YELLOW], card.cost[RED]);
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
    for (int i = 0; i < game->deckFaceup.amount; i++) {
        display_card(game->deckFaceup.cards[i]);

    }
}

void buy_card(Game *game, Player *player, int index) {
    Deck newDeck;
    newDeck.cards = malloc(0);
    newDeck.amount = 0;
    // player->hand.cards = realloc(player->hand.cards,
    //         sizeof(Card) * (player->hand.amount + 1));
    // player->hand.cards[player->hand.amount] = game->deckFaceup.cards[index];
    // player->hand.amount++;
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

char *listen(Player player) {
    char *message = malloc(sizeof(char) * MAX_INPUT);
    if (fgets(message, MAX_INPUT, player.output) == NULL) {
        // printf("MESSAGE FROM PLAYER IS NULL\n");
        exit_with_error(PROTOCOL_ERR);
    }
    if (message[strlen(message) - 1] != '\0') {
        message[strlen(message) - 1] = '\0';
    }
    return message;
}

int use_tokens(Game *game, Player *player, Card card, int tokens[5]) {
    // printf("Player %c purchased\n", player->id + 'A');
    int wild = tokens[WILD];
    for (int i = 0; i < 4; i++) {
        if (player->tokens[i] < tokens[i]) {
            return 0;
        }
    }
    int usedWild = 0, discountedPrice;
    for (int i = 0; i < 4; i++) {
        discountedPrice = card.cost[i] - player->currentDiscount[i];
        if (tokens[i] < discountedPrice) {
            int wildRequired = discountedPrice - tokens[i];
            if (wildRequired > (wild - usedWild)) {
        // printf("****HUB: fail here diff %i, %i - %i\n", wildRequired, wild, usedWild);
                return 0;
            } else {
                usedWild += wildRequired;
            }
        }
    }
    for (int i = 0; i < 4; i++) {
        player->tokens[i] -= tokens[i];
        game->tokenPile[i] += tokens[i];
    }
    player->wildTokens -= wild;
    return 1;
}

int process_purchase(Game *game, Player *player, char *encoded) {
    if (!match_seperators(encoded, 1, 4)) {
        return 0;
    }
    //printf("encoded: %s\n", encoded);
    char **purchaseDetails = split(encoded, "e");
    //printf("%s %s\n", purchaseDetails[0], purchaseDetails[1]);
    if (strcmp(purchaseDetails[0], "purchas") != 0 ||
            strlen(purchaseDetails[1]) < 11) {
        return 0;            
    }
    char **colSplit = split(purchaseDetails[1], ":");
    char **commaSplit = split(colSplit[1], ",");
    for (int i = 0; i < strlen(colSplit[1]); i++) {
        if (colSplit[1][i] != ',' && colSplit[1][i] != '\n'
                && !isdigit(colSplit[1][i])) {
            return 0;
        }
    }
    if (atoi(colSplit[0]) > 7) {
        return 0;
    }
    int index = atoi(colSplit[0]);
    //printf("before use_tok player %i tokens: %i %i %i %i\n", player->id, player->tokens[0], player->tokens[1], player->tokens[2], player->tokens[3]);
    int tokens[5];
    for (int i = 0; i < 5; i++) {
        tokens[i] = atoi(commaSplit[i]);
    }
    Card card = game->deckFaceup.cards[index];
    free(commaSplit);
    int status = use_tokens(game, player, card, tokens);
    free(colSplit);
    free(purchaseDetails);
    //printf("token status: %i\n", status);
    if (!status) {
        return 0;
    }
    update_discount(card.colour, player);
    player->points += card.value;
    buy_card(game, player, index);
    //printf("tokens: %i %i %i %i\n", game->tokenPile[0], game->tokenPile[1], game->tokenPile[2], game->tokenPile[3]);
    printf("Player %c purchased %i using %i,%i,%i,%i,%i\n", player->id + 'A',
            index, tokens[PURPLE], tokens[BLUE], tokens[YELLOW], tokens[RED],
            tokens[WILD]);
    send_all(game, "purchased%c:%i:%i,%i,%i,%i,%i\n", player->id + 'A', index,
            tokens[PURPLE], tokens[BLUE], tokens[YELLOW], tokens[RED], 
                tokens[WILD]);
    return 1;
}

int check_take(Game *game, char *content) {
    if (!match_seperators(content, 0, 3)) {
        return 0;
    }
    for (int i = 1; i < strlen(content); i++) {
        if (content[i] != ',' && content[i] != '\n'
                && !isdigit(content[i])) {
            return 0;
        }
    }
    char *contentCopy = malloc(sizeof(char) * (strlen(content) + 1));
    strcpy(contentCopy, content);
    char **commaSplit = split(contentCopy, ",");
    for (int i = 0; i < 4; i++) {
        if (strcmp(commaSplit[i], "") == 0 || atoi(commaSplit[i]) > game->tokenPile[i]) {
            free(commaSplit);
            free(contentCopy);
            return 0;
        }
    }
    free(commaSplit);
    free(contentCopy);
    return 1;
}

int process_take(Game *game, Player *player, char *encoded) {
    char **takeDetails = split(encoded, "e");
    if (strcmp(takeDetails[0], "tak") != 0 || !check_take(game, takeDetails[1])) {
        free(takeDetails);
        return 0;
    }
    char **commaSplit = split(takeDetails[1], ",");
    int tokenPurple = 0, tokenBlue = 0, tokenYellow = 0, tokenRed = 0;
    for (int i = 0; i < 4; i++) {
        game->tokenPile[i] -= atoi(commaSplit[i]);
        player->tokens[i] += atoi(commaSplit[i]);
        // printf("HUB: player %i add %i:%i, total %i\n", player->id, i, atoi(commaSplit[i]), player->tokens[i]);
        switch (i) {
            case (PURPLE):
                tokenPurple = atoi(commaSplit[i]);
                break;
            case (BLUE):
                tokenBlue = atoi(commaSplit[i]);
                break;
            case (YELLOW):
                tokenYellow = atoi(commaSplit[i]);
                break;
            case (RED):
                tokenRed = atoi(commaSplit[i]);
                break;
        }
    }
    free(commaSplit);
    free(takeDetails);
    send_all(game, "took%c:%i,%i,%i,%i\n", player->id + 'A',
            tokenPurple, tokenBlue, tokenYellow, tokenRed);
    printf("Player %c drew %i,%i,%i,%i\n", player->id + 'A',
            tokenPurple, tokenBlue, tokenYellow, tokenRed);
    return 1;
}

int process_wild(Game *game, Player *player, char *encoded) {
    if (strlen(encoded) != 4) {
        return 0;
    }
    send_all(game, "wild%c\n", player->id + 'A');
    printf("Player %c took a wild\n", player->id + 'A');
    return 1;
}

int process(Game *game, Player *player, char *encoded) {
    // printf("***processing %s from: %c\n", encoded, player->id + 'A');
    if (strstr(encoded, "purchase") != NULL) {
        //printf("%i, attempt to purchase: %s\n", player->id, encoded);
        //fprintf(stderr, "before cost card 2 player %i tokens: %i %i %i %i\n", player->id, player->tokens[0], player->tokens[1], player->tokens[2], player->tokens[3]);
        return process_purchase(game, player, encoded);
    } else if (strstr(encoded, "take") != NULL) {
        // printf("processing %s from: %c\n", encoded, player->id + 'A');
        // printf("**processing take: %i\n", x);
        return process_take(game, player, encoded);;
    } else if (strstr(encoded, "wild") != NULL) {
        process_wild(game, player, encoded);
        return 1;
    } else {
        //printf("INVALID COMM BY PLAYER recieved %s\n", encoded);
        return 0;
    }
}

void init_game(Game *game) {
    game->deckIndex = 0;
    game->deckFaceup.amount = 0;
    draw_cards(game);
    //printf("Comm started -----\n");
    send_all(game, "tokens%i\n", game->tokenPile[PURPLE]);
    for (int i = 0; i < game->deckFaceup.amount; i++) {
        Card c = game->deckFaceup.cards[i];
        send_all(game, "newcard%c:%i:%i,%i,%i,%i\n", c.colour, c.value,
                c.cost[0], c.cost[1], c.cost[2], c.cost[3]);
    }
}

int game_is_over(Game game) {
    for (int i = 0; i < game.playerAmount; i++) {
        if (game.players[i].points >= game.winPoints) {
            return 1;
        }
    }
    return 0;
}

void play_game(Game *game) {
    init_game(game);
    // display_deck(game->deckFaceup);
    int gameOver = 0;
    while (!gameOver) {
        if (game->deckFaceup.amount == 0 || game_is_over(*game)) {
            send_all(game, "eog\n");
            get_winners(game, get_highest_points(*game), TRUE);
            break;
        }
        for (int i = 0; i < game->playerAmount; i++) {
            send_message(game->players[i], "dowhat\n");
            char *message = listen(game->players[i]);
            if (!process(game, &(game->players[i]), message)) {
                send_all(game, "eog\n");
                printf("Game ended due to disconnect\n");
            }
            free(message);
        }
    }
    send_all(game, "eog\n");
}

int main(int argc, char **argv) {
    check_args(argc, argv);
    Game game;
    game.deckTotal = load_deck(argv[DECK_FILE]);
    game.winPoints = atoi(argv[WIN_POINTS]);
    for (int i = 0; i < 4; i++) {
        game.tokenPile[i] = atoi(argv[TOKENS]);
    }
    char **playersPaths = malloc(0);
    game.playerAmount = 0;
    for (int i = 4; i < argc; i++) {
        if (i != argc) {
            playersPaths = realloc(playersPaths, sizeof(char *) * (i - 3));
        }
        playersPaths[i - 4] = argv[i];
        game.playerAmount++;
    }
    game.players = setup_players(playersPaths, game.playerAmount);
    free(playersPaths);
    play_game(&game);
    free_game(game);
}
