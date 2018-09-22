#include "shared.h"

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
