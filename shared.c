#include "shared.h"

/**
* Gets the index of an card in a deck.
* @param deck - The deck to parse.
* @param card - The card to retrieve the index of.
* @param first - Get the first matching instance or last.
* @return int - Index of the card.
*/
int index_of_card(Deck deck, Card card, int first) {
    int index = -1;
    for (int i = 0; i < deck.amount; i++) {
        Card c = deck.cards[i];
        if (c.colour == card.colour && c.value == card.value
                && c.cost[PURPLE] == card.cost[PURPLE]
                && c.cost[BROWN] == card.cost[BROWN]
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

/**
* Setup initial player values.
* @param player - The players to set the values by.
*/
void set_player_values(Player *player) {
    player->wildTokens = 0;
    player->points = 0;
    for (int i = 0; i < 4; i++) {
        player->currentDiscount[i] = 0;
        player->tokens[i] = 0;
    }
}

/**
* Check if a string is an digit.
* @param string - The string to check.
* @return int - 1 if the string contains only digits.
*/
int is_string_digit(char *string) {
    for (int i = 0; i < strlen(string); i++) {
        if (!isdigit(string[i])) {
            return 0;
        }
    }
    return 1;
}

/**
* Split an string in to an array of strings by an character.
* @param string - The string to split.
* @param character - The character to split it by.
* @return char - An array of split strings.
*/
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

/**
* Check an array of strings to ensure no empty or space characters.
* @param content - The array of strings to check.
* @param length - The length of the array/
* @return int - True if the array contains no spaces or is not empty.
*/
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

/**
* Checks an encoded card for validity.
* @param content - The encoded content.
* @return int - 1 if the encoded card is valid.
*/
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

/**
* Match the amount of seperators in a string.
* @param str - The string to parse.
* @param expectedColumn - The expected amount of columns.
* @param expectedComma - The expected amount of commas.
* @return int - 1 if the amount of seperators match.
*/
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

/**
* Updates the player discounts.
* @param colour - The colour of the discount to update.
* @param player - The player to update.
*/
void update_discount(char colour, Player *player) {
    switch (colour) {
        case ('P'):
            player->currentDiscount[PURPLE]++;
            break;
        case ('B'):
            player->currentDiscount[BROWN]++;
            break;
        case ('Y'):
            player->currentDiscount[YELLOW]++;
            break;
        case ('R'):
            player->currentDiscount[RED]++;
            break;
    }
}

/**
* Gets the highest points an player has.
* @param game - The game instance.
* @return int - Highest point of an player/
*/
int get_highest_points(Game game) {
    int highestPoint = 0;
    for (int i = 0; i < game.playerAmount; i++) {
        if (game.players[i].points >= highestPoint) {
            highestPoint = game.players[i].points;
        }
    }
    return highestPoint;
}

/**
* Get the winners of a game.
* @param game - The game instance.
* @param points - The points the players must match.
* @param isHub - Is the caller from the hub or player.
*/
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