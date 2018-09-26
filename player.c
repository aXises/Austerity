#include "player.h"

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
        // fprintf(stderr, "MESSAGE FROM HUB IS NULL\n");
        exit_with_error(COMM_ERR, "");
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

void free_game(Game *game) {
    if (game->deckFaceup.amount >= 0) {
        free(game->deckFaceup.cards);
    }
    if (game->playerAmount > 0) {
        free(game->players);
    }
}

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

Game setup_game(int amount) {
    Game game;
    game.playerAmount = amount;
    game.deckFaceup.amount = 0;
    game.deckFaceup.cards = malloc(0);
    return game;
}

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

void add_card(Deck *deck, Card card) {
    deck->cards = realloc(deck->cards, sizeof(Card) * (deck->amount + 1));
    deck->cards[deck->amount] = card;
    deck->amount++;
}

int process_newcard(Game *game, char *encoded) {
    char **cardDetails = split(encoded, "d");
    if (strcmp(cardDetails[0], "newcar") != 0 || !check_card(cardDetails[1])) {
        return 0;
    }
    // fprintf(stderr, "str1: %s str2: %s\n", cardDetails[0], cardDetails[1]);
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
    // fprintf(stderr, "current size at process new card :%i\n", game->deckFaceup.amount);
    add_card(&game->deckFaceup, card);
    free(columnSplit);
    free(commaSplit);
    free(cardDetails);
    // fprintf(stderr, "checkcard res: %i\n", check_card(s[1]));
    return 1;
}

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
    // fprintf(stderr, "player index = %i\n", playerIndex);
    char **commaSplit = split(columnSplit[1], ",");
    for (int i = 0; i < 4; i++) {
        game->tokenPile[i] -= atoi(commaSplit[i]);
        game->players[playerIndex].tokens[i] += atoi(commaSplit[i]);
    }
    free(commaSplit);
    free(columnSplit);
    free(details);
    return 1;
}
    
int process_wild(Game *game, char *encoded) {
    if (strlen(encoded) != 5 || encoded[4] > 'Z' || encoded[4] < 'A') {
        return 0;
    }
    game->players[(int) encoded[4] - 'A'].wildTokens++;
    return 1;
}

void update_tokens(Game *game, Player *player, int tokens[5]) {
    for (int i = 0; i < 4; i++) {
        player->tokens[i] -= tokens[i];
        game->tokenPile[i] += tokens[i];
    }
    player->wildTokens -= tokens[WILD];
}

int process_purchase(Game *game, char *encoded) {
    char **details = split(encoded, "d");
    char **colSplit = split(details[1], ":");
    char **commaSplit = split(colSplit[2], ",");
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

void display_stats(Game *game) {
    for (int i = 0; i < game->deckFaceup.amount; i++) {
        Card c = game->deckFaceup.cards[i];
        fprintf(stderr, "Card %i:%c/%i/%i,%i,%i,%i\n", i, c.colour, c.value,
            c.cost[PURPLE], c.cost[BLUE], c.cost[YELLOW], c.cost[RED]);
    }
    for (int i = 0; i < game->playerAmount; i++) {
        Player p = game->players[i];
        fprintf(stderr, "Player %c:%i:Discounts=%i,%i,%i,%i:Tokens=%i,%i,%i,%i"\
                ",%i\n", p.id + 'A', p.points, p.currentDiscount[PURPLE],
                p.currentDiscount[BLUE], p.currentDiscount[YELLOW],
                p.currentDiscount[RED], p.tokens[PURPLE], p.tokens[BLUE],
                p.tokens[YELLOW], p.tokens[RED], p.wildTokens);
    } 
}

int process(Game *game, Player *player, char *encoded) {
    int status;
    if (strstr(encoded, "dowhat") != NULL) {
        // fprintf(stderr, "player %c has %i wild\n", player->id, player->wildTokens);
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
        display_stats(game);
    } else if (strstr(encoded, "newcard") != NULL) {
        // fprintf(stderr, "newcard\n");
        status = process_newcard(game, encoded);
        display_stats(game);
    } else if (strstr(encoded, "purchased") != NULL) {
        status = process_purchase(game, encoded);
        display_stats(game);
    } else if (strstr(encoded, "took") != NULL) {
        // fprintf(stderr, "processing took\n");
        status = process_took(game, encoded);
        display_stats(game);
    } else if (strstr(encoded, "wild") != NULL) {
        // fprintf(stderr, "processing wild\n");
        status = process_wild(game, encoded);
        display_stats(game);
    } else {
        status = 0;
    }
    return status;
}

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

void play_game(char *amount, char *id, char *name) {
    Game game = setup_game(atoi(amount));
    game.players = setup_players(atoi(amount));
    while (1) {
        char *message = listen();
        if (strcmp(message, "eog") == 0) {
            free(message);
            get_winners(&game, get_highest_points(game), FALSE);
            break;
        }
        if (!process(&game, &game.players[atoi(id)], message)) {
            free(message);
            break;
        };
        free(message);
    }
    free_game(&game);
    // fprintf(stderr, "player %s shutdown\n", id);
}