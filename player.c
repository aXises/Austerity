#include "player.h"

/**
*
* @param error
* @param name
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
*
* @param argc
* @param argv
* @param name
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
*
* @param message
*/
void send_message(char *message, ...) {
    va_list args;
    va_start(args, message);
    vprintf(message, args);
    va_end(args);
    fflush(stdout);
}

/**
*
* @return char
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
*
* @param encoded
* @return int
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
*
* @param game
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
*
* @param id
* @param name
* @return Player
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
*
* @param amount
* @return Game
*/
Game setup_game(int amount) {
    Game game;
    game.playerAmount = amount;
    game.deckFaceup.amount = 0;
    game.deckFaceup.cards = malloc(0);
    return game;
}

/**
*
* @param deck
* @param index
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
*
* @param deck
* @param card
* adds 1 card to the deck.
*/
void add_card(Deck *deck, Card card) {
    deck->cards = realloc(deck->cards, sizeof(Card) * (deck->amount + 1));
    deck->cards[deck->amount] = card;
    deck->amount++;
}

/**
*
* @param game
* @param encoded
* @return int
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
    // fprintf(stderr, "current size at process new card :%i\n", game->deckFaceup.amount);
    add_card(&game->deckFaceup, card);
    free(columnSplit);
    free(commaSplit);
    free(cardDetails);
    // fprintf(stderr, "checkcard res: %i\n", check_card(s[1]));
    return 1;
}

/**
*
* @param encoded
* @param game
* @return int
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
*
* @param game
* @param encoded
* @return int
*/
int process_wild(Game *game, char *encoded) {
    if (strlen(encoded) != 5 || encoded[4] > 'Z' || encoded[4] < 'A') {
        return 0;
    }
    game->players[(int) encoded[4] - 'A'].wildTokens++;
    return 1;
}

/**
*
* @param game
* @param player
* @param tokens
*/
void update_tokens(Game *game, Player *player, int tokens[5]) {
    for (int i = 0; i < 4; i++) {
        player->tokens[i] -= tokens[i];
        game->tokenPile[i] += tokens[i];
    }
    player->wildTokens -= tokens[WILD];
}

/**
*
* @param game
* @param encoded
* @return int
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
*
* @param game
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
*
* @param game
* @param player
* @param encoded
* @return int
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
*
* @param amount
* @return Player
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
*
* @param amount
* @param id
* @param name
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
*
* @param deck
* @return int
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
*
* @param deck
* @param value
* @return Deck
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
*
* @param card
* @param player
* @return int
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
*
* @param deck
* @param player
* @return Deck
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
*
* @param card
* @param player
* @param finalCost
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
*
* @param cost
* @return int
*/
int sum_cost(int cost[5]) {
    int sum = 0;
    for (int i = 0; i < 5; i++) {
        sum += cost[i];
    }
    return sum;
}

/**
*
* @param deck
* @param player
* @param costTotal
* @return Deck
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
*
* @param game
* @param player
* @return int
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