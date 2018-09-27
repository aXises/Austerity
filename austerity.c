#include "austerity.h"

/**
* Frees memory allocated for the game.
* @param game - The instance of the game.
*/
void free_game(Game *game) {
    if (game->deckTotal.amount < 8) {
        free(game->deckTotal.cards);
    } else {
        if (game->deckTotal.amount > 0) {
            free(game->deckTotal.cards);
        }
        if (game->deckFaceup.amount > 0) {
            free(game->deckFaceup.cards);
        }
    }
    for (int i = 0; i < game->playerAmount - 1; i++) {
        fclose(game->players[i].input);
        fclose(game->players[i].output);
    }
    free(game->players);
}

/**
* Kills child processes in the game.
* @param game - The instance of the game.
*/
void kill_children(Game *game) {
    for (int i = 0; i < game->playerAmount; i++) {
        if (game->players[i].pid != 0) {
            kill(game->players[i].pid, SIGKILL);
        }
    }
}

/**
* Wait for children to exit and print the exit status.
* @param game - The instance of the game.
*/
void wait_children(Game *game) {
    for (int i = 0; i < game->playerAmount; i++) {
        int status, id;
        waitpid(game->players[i].pid, &status, 0);
        id = game->players[i].id;
        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) > 0) {
                printf("Player %i ended with status %i\n", id, WEXITSTATUS(status));
            }
            continue;
        }
        if (WIFSIGNALED(status)) {
            printf("Player %i shutdown after receiving signal %i\n", id, WTERMSIG(status));
        }
    }
}

/**
* Checks arguments to the start the hub.
* @param argc - Argument counter.
* @param argv - Argument vector.
*/
void check_args(int argc, char **argv) {
    if (argc < 6 || argc > 26) {
        exit_with_error(NULL, WRONG_ARG_NUM);
    }
    for (int i = 1; i < 3; i++) {
        if (!is_string_digit(argv[i])) {
            exit_with_error(NULL, BAD_ARG);
        }
    }
}

/**
* Exit the hub with an error code.
* @param game - The game instance.
* @param error - The error code.
*/
void exit_with_error(Game *game, int error) {
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

/**
* Parse a deck and a array of string encoding of cards.
* @param deck The deck to load the cards in to.
* @param cardArr The array of cards to parse.
*/
void parse_deck(Deck deck, char **cardArr) {
    for (int i = 0; i < deck.amount; i++) {
        if (i != deck.amount - 1 || strcmp(cardArr[i], "") != 0) {
            if (!check_card(cardArr[i])) {
                exit_with_error(NULL, INVALID_DECK_FILE);
            }
            char **colSplit = split(cardArr[i], ":");
            Card card;
            if (strcmp(colSplit[1], "") == 0) {
                exit_with_error(NULL, INVALID_DECK_FILE); 
            }
            card.colour = colSplit[0][0];
            card.value = atoi(colSplit[1]);
            char **commaSplit = split(colSplit[2], ",");
            for (int j = 0; j < 4; j++) {
                if (strcmp(commaSplit[j], "") == 0) {
                    exit_with_error(NULL, INVALID_DECK_FILE); 
                }
                card.cost[j] = atoi(commaSplit[j]);
            }
            deck.cards[i] = card;
            free(colSplit);
            free(commaSplit);
        }
    }
}

/**
* Loads a deckfile.
* @param fileName the name of the deckfile.
* @return Deck a loaded deck struct.
*/
Deck load_deck(char *fileName) {
    FILE *file = fopen(fileName, "r");
    if (file == NULL || !file) {
        exit_with_error(NULL, CANNOT_ACCESS_DECK_FILE);
    }
    char *content = malloc(sizeof(char)), character;
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
    if (strcmp(content, "") == 0) {
        exit_with_error(NULL, INVALID_DECK_FILE); 
    }
    char **cardArr = split(content, "\n");
    Deck deck;
    deck.amount = cards + 1;
    deck.cards = malloc(sizeof(Card) * deck.amount);
    parse_deck(deck, cardArr);
    free(content);
    free(cardArr);
    return deck;
}

/**
* Sets up the parent process to communicate with child.
* @param game - The game instance.
* @param id the - id of the player.
* @param input - The input pipe.
* @param output  - The output pipe.
* @param test - The test pipe.
*/
void setup_parent(Game *game, int id, int input[2],
        int output[2], int test[2]) {
    if (close(input[READ]) == -1 || close(output[WRITE]) == -1 ||
            close(test[WRITE]) == -1) {
        exit_with_error(game, BAD_START);
    }
    game->players[id].input = fdopen(input[WRITE], "w");
    game->players[id].output = fdopen(output[READ], "r");
    FILE *testPipe = fdopen(test[READ], "r");
    if (game->players[id].input == NULL ||
            game->players[id].output == NULL || testPipe == NULL) {
        exit_with_error(game, BAD_START);
    }
    char *message = malloc(sizeof(char) * MAX_INPUT);
    if (fgets(message, MAX_INPUT, testPipe) != NULL) {
        fclose(testPipe);
        free(message);
        exit_with_error(game, BAD_START);
    }
    fclose(testPipe);
    free(message);

}

/**
* Setup child process to communicate with parent.
* @param game - The game instance.
* @param id - id of the player.
* @param input - The input pipe.
* @param output  - The output pipe.
* @param test - The test pipe.
* @param file - The file to execute for the child.
*/
void setup_child(Game *game, int id, int input[2], int output[2],
        int test[2], char *file) {
    if (close(input[WRITE]) == -1) {
        exit_with_error(game, BAD_START);
    }
    if (dup2(input[READ], STDIN_FILENO) == -1) {
        exit_with_error(game, BAD_START);
    }
    if (close(input[READ]) == -1) {
        exit_with_error(game, BAD_START);
    }
    if (close(output[READ]) == -1) {
        exit_with_error(game, BAD_START);
    }
    if (dup2(output[WRITE], STDOUT_FILENO) == -1) {
        exit_with_error(game, BAD_START);
    }
    if (close(output[WRITE]) == -1) {
        exit_with_error(game, BAD_START);
    }
    int devNull = open("/dev/null", O_WRONLY);
    if (devNull == -1) {
        exit_with_error(game, BAD_START);
    }
    if (dup2(devNull, STDERR_FILENO) == -1) {
        exit_with_error(game, BAD_START);
    }
    if (close(test[READ]) == -1) {
        exit_with_error(game, BAD_START);
    }
    int flags = fcntl(test[WRITE], F_GETFD);
    if (flags == -1) {
        exit_with_error(game, BAD_START);
    }
    if (fcntl(test[WRITE], F_SETFD, flags | FD_CLOEXEC) == -1) {
        exit_with_error(game, BAD_START);
    }
    char playerAmountArg[3], playerIdArg[3];
    playerAmountArg[2] = '\0', playerIdArg[2] = '\0';
    sprintf(playerAmountArg, "%d", game->playerAmount);
    sprintf(playerIdArg, "%d", id);
    execlp(file, file, playerAmountArg, playerIdArg, NULL);
    FILE *testPipe = fdopen(test[WRITE], "w");
    fprintf(testPipe, "test\n");
    fclose(testPipe);
    close(test[WRITE]);
}

/**
* Sets up a single player in the game.
* @param game - The game instance.
* @param id - The player id.
* @param file - The file name to execute.
*/
void setup_player(Game *game, int id, char *file) {
    int input[2], output[2], test[2];
    if (pipe(input) != 0 || pipe(output) != 0 || pipe(test) != 0) {
        exit_with_error(game, BAD_START);
    }
    pid_t pid = fork();
    if (pid < 0) {
        exit_with_error(game, BAD_START);
    } else if (pid > 0) { // Parent Process
        setup_parent(game, id, input, output, test);
    } else { // Child Process
        game->players[id].pid = pid;
        setup_child(game, id, input, output, test, file);
    }
}

/**
* Sets up all the players in game.
* @param game - The game instance.
* @param playerPaths - An array of string of paths to the player.
*/
void setup_players(Game *game, char **playerPaths) {
    game->players = malloc(0);
    for (int i = 0; i < game->playerAmount; i++) {
        if (i != game->playerAmount) {
            game->players = realloc(game->players, sizeof(Player) * (i + 1));
        }
        Player player;
        player.id = i;
        game->players[i] = player;
        set_player_values(&game->players[i]);
        setup_player(game, i, playerPaths[i]);
        
    }
}

/**
* Sends an message to one player.
* @param player - The player to send the message to.
* @param message - the content to send
*/
void send_message(Player player, char *message, ...) {
    va_list args;
    va_start(args, message);
    vfprintf(player.input, message, args);
    va_end(args);
    fflush(player.input);
}

/**
* Send an message to all the players.
* @param game - The game instance.
* @param message - The message to send.
*/
void send_all(Game *game, char *message, ...) {
    for (int i = 0; i < game->playerAmount; i++) {
        va_list args;
        va_start(args, message);
        vfprintf(game->players[i].input, message, args);
        fflush(game->players[i].input);
        va_end(args);
    }
}

/**
* Determine whether if a new card can be flipped.
* @param game - The game instance.
* @return int - 1 if a new card can be flipped.
* 
*/
int has_next_card(Game *game) {
    return game->deckTotal.amount != game->deckIndex;
}

/**
* Displays card detail to stdout.
* @param c - The card to display.
*/
void display_card(Card c) {
    printf("New card = Bonus %c, worth %i, costs %i,%i,%i,%i\n", c.colour, 
            c.value, c.cost[PURPLE], c.cost[BROWN],
            c.cost[YELLOW], c.cost[RED]);
}

/**
* Draws the next card and places it in to the market.
* @param game - The game instance.
* @param deck - The deck to place in to.
*/
void draw_next(Game *game, Deck *deck) {
    if (has_next_card(game)) {
        Card card = game->deckTotal.cards[(game->deckIndex) - 1];
        game->deckIndex++;
        deck->cards[7] = card;
        deck->amount++;
        display_card(card);
        send_all(game, "newcard%c:%i:%i,%i,%i,%i\n", card.colour, card.value,
                card.cost[PURPLE], card.cost[BROWN],
                card.cost[YELLOW], card.cost[RED]);
    }
}

/**
* Draws cards and places them in to the market.
* @param game - The game instance.
*/
void draw_cards(Game *game) {
    if (game->deckTotal.amount < 8) { //less than 8 cards total
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

/**
* Attempt to buy a card from the market.
* @param game - The game instance.
* @param player - The player which bought this card.
* @param index - The index of the card bought.
*/
void buy_card(Game *game, Player *player, int index) {
    Deck newDeck;
    newDeck.cards = malloc(0);
    newDeck.amount = 0;
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

/**
* Listen on an player's output pipe for an message.
* @param player - The player to listen to.
* @return char - The message sent by the player.
*/
char *listen(Player player) {
    char *message = malloc(sizeof(char) * MAX_INPUT);
    if (fgets(message, MAX_INPUT, player.output) == NULL) {
        // printf("MESSAGE FROM PLAYER IS NULL\n");
        exit_with_error(NULL, PROTOCOL_ERR);
    }
    if (message[strlen(message) - 1] != '\0') {
        message[strlen(message) - 1] = '\0';
    }
    return message;
}

/**
* Use an player's tokens to purchase an card.
* @param game - The game instance.
* @param player - The player who used the tokens.
* @param card - The card to apply the tokens to.
* @param tokens - The amount of tokens spent.
* @return int - 1 if successfully applied the tokens.
*/
int use_tokens(Game *game, Player *player, Card card, int tokens[5]) {
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

/**
* Processes the purchase command from players.
* @param game - The game instance.
* @param player - The player who sent the command.
* @param encoded - The encoded command which was sent.
* @return int - 1 if successfully purchased.
*/
int process_purchase(Game *game, Player *player, char *encoded) {
    if (!match_seperators(encoded, 1, 4)) {
        return 0;
    }
    char **purchaseDetails = split(encoded, "e");
    if (strcmp(purchaseDetails[0], "purchas") != 0 ||
            strlen(purchaseDetails[1]) < 11) {
        return 0;            
    }
    char **colSplit = split(purchaseDetails[1], ":");
    char **commaSplit = split(colSplit[1], ",");
    if (!check_encoded(commaSplit, 5)) {
        return 0;
    }
    if (atoi(colSplit[0]) > 7) {
        return 0;
    }
    int index = atoi(colSplit[0]);
    int tokens[5];
    for (int i = 0; i < 5; i++) {
        tokens[i] = atoi(commaSplit[i]);
    }
    Card card = game->deckFaceup.cards[index];
    free(commaSplit);
    int status = use_tokens(game, player, card, tokens);
    free(colSplit);
    free(purchaseDetails);
    if (!status) {
        return 0;
    }
    update_discount(card.colour, player);
    player->points += card.value;
    printf("Player %c purchased %i using %i,%i,%i,%i,%i\n", player->id + 'A',
            index, tokens[PURPLE], tokens[BROWN], tokens[YELLOW], tokens[RED],
            tokens[WILD]);
    buy_card(game, player, index);
    send_all(game, "purchased%c:%i:%i,%i,%i,%i,%i\n", player->id + 'A', index,
            tokens[PURPLE], tokens[BROWN], tokens[YELLOW], tokens[RED], 
            tokens[WILD]);
    return 1;
}

/**
* Check whether if the take action is valid.
* @param game - The game instance.
* @param content - The content of the message.
* @return int - 1 if the action is valid.
*/
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
        if (strcmp(commaSplit[i], "") == 0 || atoi(commaSplit[i]) != 0) {
            if (atoi(commaSplit[i]) > game->tokenPile[i]) {
                free(commaSplit);
                free(contentCopy);
                return 0;
            }
        }
    }
    free(commaSplit);
    free(contentCopy);
    return 1;
}

/**
* Process an player taking tokens.
* @param game - The game instance.
* @param player - The player which is performing this action.
* @param encoded - The content of the message.
* @return int - 1 if the action is valid.
*/
int process_take(Game *game, Player *player, char *encoded) {
    char **takeDetails = split(encoded, "e");
    if (strcmp(takeDetails[0], "tak") != 0 ||
            !check_take(game, takeDetails[1])) {
        free(takeDetails);
        return 0;
    }
    char **commaSplit = split(takeDetails[1], ",");
    if (!check_encoded(commaSplit, 4)) {
        return 0;
    }
    int tokenPurple = 0, tokenBROWN = 0, tokenYellow = 0, tokenRed = 0;
    for (int i = 0; i < 4; i++) {
        game->tokenPile[i] -= atoi(commaSplit[i]);
        player->tokens[i] += atoi(commaSplit[i]);
        switch (i) {
            case (PURPLE):
                tokenPurple = atoi(commaSplit[i]);
                break;
            case (BROWN):
                tokenBROWN = atoi(commaSplit[i]);
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
            tokenPurple, tokenBROWN, tokenYellow, tokenRed);
    printf("Player %c drew %i,%i,%i,%i\n", player->id + 'A',
            tokenPurple, tokenBROWN, tokenYellow, tokenRed);
    return 1;
}

/**
* Process taking an while token
* @param game - The game instance.
* @param player - The player which is performing this action.
* @param encoded - The content of the message.
* @return int - 1 if the action is valid.
*/
int process_wild(Game *game, Player *player, char *encoded) {
    if (strlen(encoded) != 4) {
        return 0;
    }
    send_all(game, "wild%c\n", player->id + 'A');
    printf("Player %c took a wild\n", player->id + 'A');
    return 1;
}

/**
* Process messages sent by the player.
* @param game - The game instance.
* @param player - The player which is performing this action.
* @param encoded - The content of the message.
* @return int - 1 if the action was successfully performed.
*/
int process(Game *game, Player *player, char *encoded) {
    if (strstr(encoded, "purchase") != NULL) {
        return process_purchase(game, player, encoded);
    } else if (strstr(encoded, "take") != NULL) {
        return process_take(game, player, encoded);
    } else if (strstr(encoded, "wild") != NULL) {
        process_wild(game, player, encoded);
        return 1;
    } else {
        return 0;
    }
}

/**
* Initializes the game.
* @param game - The game to initialize.
*/
void init_game(Game *game) {
    draw_cards(game);
    send_all(game, "tokens%i\n", game->tokenPile[PURPLE]);
    for (int i = 0; i < game->deckFaceup.amount; i++) {
        Card c = game->deckFaceup.cards[i];
        send_all(game, "newcard%c:%i:%i,%i,%i,%i\n", c.colour, c.value,
                c.cost[0], c.cost[1], c.cost[2], c.cost[3]);
    }
}

/**
* Check if the game is over.
* @param game - The game instance to check if it is over.
* @return int - 1 if the game has ended.
*/
int game_is_over(Game game) {
    for (int i = 0; i < game.playerAmount; i++) {
        if (game.players[i].points >= game.winPoints) {
            return 1;
        }
    }
    return 0;
}

/**
* Begin playing the game.
* @param game - The game instance to play.
*/
void play_game(Game *game) {
    init_game(game);
    int gameOver = 0;
    while (1) {
        if (gameOver) {
            break;
        }
        for (int i = 0; i < game->playerAmount; i++) {
            if (game->deckFaceup.amount == 0 || game_is_over(*game)) {
                send_all(game, "eog\n");
                get_winners(game, get_highest_points(*game), TRUE);
                gameOver = 1;
                break;
            }
            send_message(game->players[i], "dowhat\n");
            char *message = listen(game->players[i]);
            if (!process(game, &(game->players[i]), message)) {
                send_message(game->players[i], "dowhat\n");
                char *reprompt = listen(game->players[i]);
                if (!process(game, &(game->players[i]),reprompt)) {
                    free(reprompt);
                    send_all(game, "eog\n");
                    printf("Game ended due to disconnect\n");
                }
            }
            free(message);
        }
    }
    send_all(game, "eog\n");
}

/** sigint handler */
void sigint_handle(int sig) {
    exit_with_error(NULL, NORMAL);
}

/** Setup signal handlers */
void setup_signal_action(void) {
    struct sigaction signalInt;
    signalInt.sa_handler = sigint_handle;
    sigaction(SIGINT, &signalInt, 0);
    struct sigaction signalPipe;
    signalPipe.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &signalPipe, 0);
}

/** Main */
int main(int argc, char **argv) {
    // setup_signal_action(); // Causes many memory leaks.
    check_args(argc, argv);
    Game game;
    game.winPoints = atoi(argv[WIN_POINTS]);
    for (int i = 0; i < 4; i++) {
        game.tokenPile[i] = atoi(argv[TOKENS]);
    }
    char **playersPaths = malloc(0);
    game.playerAmount = 0;
    game.deckFaceup.amount = 0;
    game.deckIndex = 0;
    for (int i = 4; i < argc; i++) {
        if (i != argc) {
            playersPaths = realloc(playersPaths, sizeof(char *) * (i - 3));
        }
        playersPaths[i - 4] = argv[i];
        game.playerAmount++;
    }
    setup_players(&game, playersPaths);
    free(playersPaths);
    game.deckTotal = load_deck(argv[DECK_FILE]);
    play_game(&game);
    // wait_children(&game);
    free_game(&game);
}
