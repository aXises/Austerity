#include "util.h"

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