#include "player.h"

/**
* Exit the program with an error.
* @param error - The error code.
* @param name - The name of the player.
*/
void exit_with_error(int error, char *name) {
    switch(error) {
        case (WRONG_ARG_NUM):
            fprintf(stderr, "Usage: %s pcount myid\n", name);
            break;
        case (INVALID_PLAYER_COUNT):
            fprintf(stderr, "Invalid player count\n");
            break;
        case (INVALID_ID):
            fprintf(stderr, "Invalid player ID\n");
            break;
        case (COMM_ERR):
            fprintf(stderr, "Communication Error\n");
            break;
        default:
            break;
    }
    exit(error);
}

/**
* Checks arguments to the start the hub.
* @param argc - Argument counter.
* @param argv - Argument vector.
* @param name - The name of the player.
*/
void check_args(int argc, char **argv, char *name) {
    if (argc != 3) {
        exit_with_error(WRONG_ARG_NUM, name);
    }
    if (!is_string_digit(argv[TOTAL_PLAYERS]) || atoi(argv[TOTAL_PLAYERS]) > 26
            || atoi(argv[TOTAL_PLAYERS]) < 2) {
        exit_with_error(INVALID_PLAYER_COUNT, name);
    }
    if (!is_string_digit(argv[PLAYER_ID]) || atoi(argv[PLAYER_ID]) > 26
            || atoi(argv[PLAYER_ID]) < 0) {
        exit_with_error(INVALID_ID, name);
    }
    if (atoi(argv[PLAYER_ID]) >= atoi(argv[TOTAL_PLAYERS])) {
        exit_with_error(INVALID_ID, name);
    }

}

/**
* Sends an message to the hub.
* @param message The message to send.
*/
void send_message(char *message, ...) {
    va_list args;
    va_start(args, message);
    vprintf(message, args);
    va_end(args);
    fflush(stdout);
}

/**
* Listen on stdin for an message from the hub.
* @return char - The message from the hub.
*/
char *listen(void) {
    char *message = malloc(sizeof(char) * MAX_INPUT);
    if (fgets(message, MAX_INPUT, stdin) == NULL) {
        exit_with_error(COMM_ERR, "");
    }
    if (message[strlen(message) - 1] != '\0') {
        message[strlen(message) - 1] = '\0';
    }
    return message;
}

/**
* Processes the token message.
* @param encoded - The encoded message.
* @return int - 1 if successfully processed.
*/
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

/**
* Frees memory allocated to the game.
* @param game - The game instance to free.
*/
void free_game(Game *game) {
    if (game->deckFaceup.amount >= 0) {
        free(game->deckFaceup.cards);
    }
    if (game->playerAmount > 0) {
        free(game->players);
    }
}

/**
* Sets up an player
* @param id - The id of the player.
* @param name - The name of the player.
* @return Player - A initial state of a player.
*/
Player setup_player(int id, char *name) {
    Player player;
    player.id = id;
    player.name = name;
    for (int i = 0; i < 4; i++) {
        player.currentDiscount[i] = 0;
        player.tokens[i] = 0;
    }
    player.wildTokens = 0;
    return player;
}

/**
* Sets up an game.
* @param amount - The total amount of players.
* @return Game - An initial state of a game.
*/
Game setup_game(int amount) {
    Game game;
    game.playerAmount = amount;
    game.deckFaceup.amount = 0;
    game.deckFaceup.cards = malloc(0);
    return game;
}

/**
* Removes a card from an deck by its index.
* @param deck - The deck to remove from.
* @param index - The index of the card.
*/
void remove_card(Deck *deck, int index) {
    deck->amount--;
    Deck newDeck;
    newDeck.amount = deck->amount;
    newDeck.cards = malloc(sizeof(Card) * newDeck.amount);
    for (int i = 0; i < newDeck.amount; i++) {
        if (i < index) {
            newDeck.cards[i] = deck->cards[i];
        } else {
            newDeck.cards[i] = deck->cards[i + 1];
        }
    }
    free(deck->cards);
    deck->cards = newDeck.cards;
}

/**
* Add a card to an deck.
* @param deck - The deck to add to.
* @param card - The card to add.
*/
void add_card(Deck *deck, Card card) {
    deck->cards = realloc(deck->cards, sizeof(Card) * (deck->amount + 1));
    deck->cards[deck->amount] = card;
    deck->amount++;
}

/**
* Processes the newcard message from the hub.
* @param game - The game instance.
* @param encoded - The encoded message.
* @return int - 1 if successfully processed.
*/
int process_newcard(Game *game, char *encoded) {
    char **cardDetails = split(encoded, "d");
    if (strcmp(cardDetails[0], "newcar") != 0 || !check_card(cardDetails[1])) {
        return 0;
    }
    char **columnSplit = split(cardDetails[1], ":");
    char **commaSplit = split(columnSplit[2], ",");
    if (!check_encoded(commaSplit, 4)) {
        return 0;
    }
    Card card;
    card.colour = columnSplit[0][0];
    card.value = atoi(columnSplit[1]);
    for (int i = 0; i < 4; i++) {
        if (strcmp(commaSplit[i], "") == 0) {
            return 0;
        }
        card.cost[i] = atoi(commaSplit[i]);
    }
    add_card(&game->deckFaceup, card);
    free(columnSplit);
    free(commaSplit);
    free(cardDetails);
    return 1;
}

/**
* Process an take token action.
* @param game - The game instance.
* @param encoded - The content of the message.
* @return int - 1 if successfully processed.
*/
int process_took(Game *game, char *encoded) {
    char **details = split(encoded, "k");
    if (strcmp(details[0], "too") != 0 || strlen(details[0]) != 3 ||
            details[1][0] > 'Z' || details[1][0] < 'A') {
        return 0;
    }
    if (!match_seperators(details[1], 1, 3)) {
        return 0;
    }
    char **columnSplit = split(details[1], ":");
    int playerIndex = (int) columnSplit[0][0] - 'A';
    char **commaSplit = split(columnSplit[1], ",");
    if (!check_encoded(commaSplit, 4)) {
        return 0;
    }
    for (int i = 0; i < 4; i++) {
        game->tokenPile[i] -= atoi(commaSplit[i]);
        game->players[playerIndex].tokens[i] += atoi(commaSplit[i]);
    }
    free(commaSplit);
    free(columnSplit);
    free(details);
    return 1;
}

/**
* Processes an wild token command.
* @param game - The game instance.
* @param encoded - The encoded message.
* @return int - 1 if successfully processed.
*/
int process_wild(Game *game, char *encoded) {
    if (strlen(encoded) != 5 || encoded[4] > 'Z' || encoded[4] < 'A') {
        return 0;
    }
    game->players[(int) encoded[4] - 'A'].wildTokens++;
    return 1;
}

/**
* Update an player's total amount of tokens.
* @param game - The game instance.
* @param player - The player who used the tokens.
* @param tokens - The amount of tokens spent.
*/
void update_tokens(Game *game, Player *player, int tokens[5]) {
    for (int i = 0; i < 4; i++) {
        player->tokens[i] -= tokens[i];
        game->tokenPile[i] += tokens[i];
    }
    player->wildTokens -= tokens[WILD];
}

/**
* Processes the purchase command from players.
* @param game - The game instance.
* @param encoded - The encoded command which was sent.
* @return int - 1 if successfully purchased.
*/
int process_purchase(Game *game, char *encoded) {
    char **details = split(encoded, "d");
    if (strcmp(details[0], "purchase") != 0 ||
            !match_seperators(details[1], 2, 4)) {
        return 0;
    }
    char **colSplit = split(details[1], ":");
    if (strlen(colSplit[0]) != 1 || colSplit[0][0] < 'A'
            || colSplit[0][0] > 'Z' || strlen(colSplit[1]) != 1
            || (int) colSplit[1][0] - '0' < 0
            || (int) colSplit[1][0] - '0' > 7) {
        return 0;
    }
    char **commaSplit = split(colSplit[2], ",");
    if (!check_encoded(commaSplit, 5)) {
        return 0;
    }
    int playerIndex = colSplit[0][0] - 'A';
    int cardIndex = atoi(colSplit[1]);
    Card card = game->deckFaceup.cards[cardIndex];
    game->players[playerIndex].points += card.value;
    int tokens[5];
    for (int i = 0; i < 5; i++) {
        tokens[i] = atoi(commaSplit[i]);
    }
    update_tokens(game, &game->players[playerIndex], tokens);
    update_discount(card.colour, &game->players[playerIndex]);
    remove_card(&game->deckFaceup, atoi(colSplit[1]));
    free(commaSplit);
    free(colSplit);
    free(details);
    return 1;
}

/**
* Display the game status.
* @param game - The game instance.
*/
void display_stats(Game *game) {
    for (int i = 0; i < game->deckFaceup.amount; i++) {
        Card c = game->deckFaceup.cards[i];
        fprintf(stderr, "Card %i:%c/%i/%i,%i,%i,%i\n", i, c.colour, c.value,
                c.cost[PURPLE], c.cost[BROWN], c.cost[YELLOW], c.cost[RED]);
    }
    for (int i = 0; i < game->playerAmount; i++) {
        Player p = game->players[i];
        fprintf(stderr, "Player %c:%i:Discounts=%i,%i,%i,%i:Tokens=%i,%i,%i,%i"\
                ",%i\n", p.id + 'A', p.points, p.currentDiscount[PURPLE],
                p.currentDiscount[BROWN], p.currentDiscount[YELLOW],
                p.currentDiscount[RED], p.tokens[PURPLE], p.tokens[BROWN],
                p.tokens[YELLOW], p.tokens[RED], p.wildTokens);
    } 
}

/**
* Process encoded messages sent from the hub.
* @param game - The game instance.
* @param player - The current active player.
* @param encoded - The encoded string.
* @return int - 1 if the encoded string was processed succcessfully.
*/
int process(Game *game, Player *player, char *encoded) {
    int status;
    if (strstr(encoded, "dowhat") != NULL) {
        fprintf(stderr, "Received dowhat\n");
        process_dowhat(game, player);
        status = 1;
    } else if (strstr(encoded, "tokens") != NULL) {
        int tokens = process_tokens(encoded);
        if (tokens == -1) {
            status = 0;
        } else {
            for (int i = 0; i < 4; i++) {
                game->tokenPile[i] = tokens;
            }
            status = 1;
        }
    } else if (strstr(encoded, "newcard") != NULL) {
        status = process_newcard(game, encoded);
    } else if (strstr(encoded, "purchased") != NULL) {
        status = process_purchase(game, encoded);
    } else if (strstr(encoded, "took") != NULL) {
        status = process_took(game, encoded);
    } else if (strstr(encoded, "wild") != NULL) {
        status = process_wild(game, encoded);
    } else {
        status = 0;
    }
    if (status && strstr(encoded, "dowhat") == NULL && strcmp(encoded, "eog") != 0) {
        display_stats(game);
    }
    return status;
}

/**
* Setup initial players.
* @param amount - The amount of players to set up.
* @return Player - An array of players.
*/
Player *setup_players(const int amount) {
    Player *players = malloc(0);
    for (int i = 0; i < amount; i++) {
        if (i != amount) {
            players = realloc(players, sizeof(Player) * (i + 1));
        }
        Player player;
        player.id = i;
        set_player_values(&player);
        players[i] = player;
    }
    return players;
}

/**
* Begin playing an game.
* @param amount - The total amount of players.
* @param id - The id of the current player.
* @param name - The name of the current player.
*/
void play_game(char *amount, char *id, char *name) {
    Game game = setup_game(atoi(amount));
    game.players = setup_players(atoi(amount));
    int error = NORMAL;
    while (1) {
        char *message = listen();
        if (strcmp(message, "eog") == 0) {
            free(message);
            get_winners(&game, get_highest_points(game), FALSE);
            break;
        }
        if (!process(&game, &game.players[atoi(id)], message)) {
            error = COMM_ERR;
            free(message);
            break;
        }
        free(message);
    }
    free_game(&game);
    exit_with_error(error, "");
}

/**
* Gets the largest point of an card in an deck.
* @param deck - The deck to parse.
* @return int - The largest point.
*/
int largest_value(Deck deck) {
    int largest = 0;
    for (int i = 0; i < deck.amount; i++) {
        if (deck.cards[i].value > largest) {
            largest = deck.cards[i].value;
        }
    }
    return largest;
}

/**
* Gets an deck of cards which match a certain value.
* @param deck - The deck to parse.
* @param value - The value to match.
* @return Deck - Deck of cards which match the value.
*/
Deck get_card_by_value(Deck deck, int value) {
    Deck newDeck;
    newDeck.cards = malloc(0);
    int counter = 0;
    for (int i = 0; i < deck.amount; i++) {
        if (deck.cards[i].value == value) {
            newDeck.cards = realloc(newDeck.cards,
                    sizeof(Card) * (counter + 1));
            newDeck.cards[counter] = deck.cards[i];
            counter++;
        }
    }
    newDeck.amount = counter;
    return newDeck;
}

/**
* Check if an player can afford the card.
* @param card - The card to check.
* @param player - The player to compare to.
* @return int - 1 if the player can afford the card.
*/
int can_afford(Card card, Player *player) {
    int wildUsed = 0;
    for (int i = 0; i < 4; i++) {
        if (card.cost[i] - player->currentDiscount[i] > player->tokens[i]) {
            wildUsed += card.cost[i] - player->currentDiscount[i] - player->tokens[i];
            if (wildUsed > player->wildTokens) {
                return 0;
            }
        }
    }
    return 1;
}

/**
* Gets the deck of cards which is affordable by an player.
* @param deck - The deck to parse.
* @param player - The active player.
* @return Deck - The deck of cards which is affordable by an player.
*/
Deck affordable_cards(Deck deck, Player *player) {
    Deck newDeck;
    newDeck.cards = malloc(0);
    int counter = 0;
    for (int i = 0; i < deck.amount; i++) {
        if (can_afford(deck.cards[i], player)) {
            newDeck.cards = realloc(newDeck.cards,
                    sizeof(Card) * (counter + 1));
            newDeck.cards[counter] = deck.cards[i];
            counter++;
        }
    }
    newDeck.amount = counter;
    return newDeck;
}

/**
* Gets the cost of an card.
* @param card - The card to get the cost of.
* @param player - The player who wants to know.
* @param finalCost - The final cost.
*/
void cost_of_card(Card card, Player *player, int finalCost[5]) {
    for (int i = 0; i < 5; i++) {
        finalCost[i] = 0;
    }
    for (int i = 0; i < 4; i++) {
        int priceAfterDiscount = card.cost[i] - player->currentDiscount[i];
        if (priceAfterDiscount > player->tokens[i]) {
            int wildUsed = priceAfterDiscount - player->tokens[i];
            finalCost[WILD] += wildUsed;
            finalCost[i] += player->tokens[i];
        } else {
            finalCost[i] += priceAfterDiscount;
        }
        if (finalCost[i] < 0) {
            finalCost[i] = 0;
        }

    }
}

/**
* Sums the total cost.
* @param cost - Array of prices to sum.
* @return int - Total sum of the array.
*/
int sum_cost(int cost[5]) {
    int sum = 0;
    for (int i = 0; i < 5; i++) {
        sum += cost[i];
    }
    return sum;
}

/**
* Gets an deck of cards which match a total cost.
* @param deck - The deck to parse.
* @param player - The active player.
* @param costTotal - The value to match.
* @return Deck - Deck of cards which match the cost.
*/
Deck get_card_by_cost(Deck deck, Player *player, int costTotal) {
    Deck newDeck;
    newDeck.cards = malloc(0);
    int counter = 0;
    for (int i = 0; i < deck.amount; i++) {
        int cost[5];
        cost_of_card(deck.cards[i], player, cost);
        if (sum_cost(cost) == costTotal) {
            newDeck.cards = realloc(newDeck.cards,
                    sizeof(Card) * (counter + 1));
            newDeck.cards[counter] = deck.cards[i];
            counter++;
        }
    }
    newDeck.amount = counter;
    return newDeck;
}

/**
* Check if an player can take tokens.
* @param game - The game instance.
* @param player - The player to check.
* @return int - 1 if tokens can be taken.
*/
int can_take_tokens(Game *game, Player *player) {
    int emptyPile = 0;
    for (int i = 0; i < 4; i++) {
        if (game->tokenPile[i] == 0) {
            emptyPile++;
        }
    }
    if (emptyPile > 1) {
        return 0;
    }
    return 1;
}


/**
* Attempt to purchase an card.
* @param game - The game instance.
* @param player - The player who wants to purchase.
* @param index - The index of the card. 
*/
void purchase_card(Game *game, Player *player, int index) {
    int cost[5];
    cost_of_card(game->deckFaceup.cards[index], player, cost);
    send_message("purchase%i:%i,%i,%i,%i,%i\n", index, cost[PURPLE],
            cost[BROWN], cost[YELLOW], cost[RED], cost[WILD]);
}