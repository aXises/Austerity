#include "shared.h"

int index_of_card(Deck deck, Card card, int first) {
    int index = -1;
    for (int i = 0; i < deck.amount; i++) {
        Card c = deck.cards[i];
        if (c.colour == card.colour && c.value == card.value
                && c.cost[PURPLE] == card.cost[PURPLE]
                && c.cost[BLUE] == card.cost[BLUE]
                && c.cost[YELLOW] == card.cost[YELLOW]
                && c.cost[RED] == card.cost[RED]) {
            index = i;
            if (first) {
                break;
            }
        }

    }
    return index;
}

void set_player_values(Player *player) {
    player->wildTokens = 0;
    player->points = 0;
    for (int i = 0; i < 4; i++) {
        player->currentDiscount[i] = 0;
        player->tokens[i] = 0;
    }
}

int is_string_digit(char *string) {
    for (int i = 0; i < strlen(string); i++) {
        if (!isdigit(string[i])) {
            return 0;
        }
    }
    return 1;
}

char **split(char *string, char *character) {
    char *segment;
    char **splitString = malloc(sizeof(char *));
    int counter = 0;
    while((segment = strsep(&string, character)) != NULL) {
        splitString = realloc(splitString, sizeof(char *) * (counter + 1));
        splitString[counter] = segment;
        counter++;
    }
    return splitString;
}

void display_deck(Deck deck) {
    for (int i = 0; i < deck.amount; i++) {
        Card c = deck.cards[i];
        fprintf(stderr, "%i: %c %i %i %i %i %i\n", i, c.colour, c.value, c.cost[0],
                c.cost[1], c.cost[2], c.cost[3]);
    }
}

int check_encoded(char **content, int length) {
    for (int i = 0; i < length; i++) {
        if (!is_string_digit(content[i]) ||
                strcmp(content[i], " ") == 0 ||
                strcmp(content[i], "") == 0) {
            return 0;
        }
    }
    return 1;
}

int check_card(char *content) {
    if (content[0] != 'B' && content[0] != 'Y' && content[0] != 'P' &&
            content[0] != 'R') {
        return 0;  
    }
    if (!match_seperators(content, 2, 3)) {
        return 0;
    }
    for (int i = 1; i < strlen(content); i++) {
        if (content[i] != ':' && content[i] != ',' && content[i] != '\n'
                && !isdigit(content[i])) {
            return 0;
        }
    }
    return 1;
}

int match_seperators(char *str, const int expectedColumn,
        const int expectedComma) {
    int colAmount = 0, commaAmount = 0;
    for (int i = 0; i < strlen(str); i++) {
        switch(str[i]) {
            case (':'):
                colAmount++;
                break;
            case (','):
                commaAmount++;
                break;
        }
    }
    return colAmount == expectedColumn && commaAmount == expectedComma; 
}

void update_discount(char colour, Player *player) {
    switch (colour) {
        case ('P'):
            player->currentDiscount[PURPLE]++;
            break;
        case ('B'):
            player->currentDiscount[BLUE]++;
            break;
        case ('Y'):
            player->currentDiscount[YELLOW]++;
            break;
        case ('R'):
            player->currentDiscount[RED]++;
            break;
    }
}

int get_highest_points(Game game) {
    int highestPoint = 0;
    for (int i = 0; i < game.playerAmount; i++) {
        if (game.players[i].points >= highestPoint) {
            highestPoint = game.players[i].points;
        }
    }
    return highestPoint;
}

void get_winners(Game *game, int points, int isHub) {
    Player *winners = malloc(0);
    int counter = 0;
    for (int i = 0; i < game->playerAmount; i++) {
        if (game->players[i].points == points) {
            winners = realloc(winners, sizeof(Player) * (counter + 1));
            winners[counter] = game->players[i];
            counter++;
        }
    }
    if (isHub) {
        printf("Winner(s) ");
    } else {
        fprintf(stderr, "Game over. Winners are ");
    }
    for (int i = 0; i < counter; i++) {
        if (isHub) {
            printf("%c", winners[i].id + 'A');
        } else {
            fprintf(stderr, "%c", winners[i].id + 'A');
        }
        if (i != counter - 1) {
            if (isHub) {
                printf(",");
            } else {
                fprintf(stderr, ",");
            }
        }
    }
    if (isHub) {
        printf("\n");
    } else {
        fprintf(stderr, "\n");
    }
    free(winners);
}