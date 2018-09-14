#include "austerity.h"
#include "shared.h"
#include "util.h"

#define READ 0
#define WRITE 1

/**
 * Function Prototypes
 **/
void check_args(int, char **);
void checkDeckFile(char **);
void exit_with_error(int);
Deck load_deck(char *);

// void checkDeckFile(char **file) {
    
// }

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
            exit(WRONG_ARG_NUM);
            break;
        case(BAD_ARG):
            fprintf(stderr, "Bad argument\n");
            exit(BAD_ARG);
            break;
        case(CANNOT_ACCESS_DECK_FILE):
            fprintf(stderr, "Cannot access deck file\n");
            exit(CANNOT_ACCESS_DECK_FILE);
            break;
        case(INVALID_DECK_FILE):
            fprintf(stderr, "Invalid deck file contents\n");
            exit(INVALID_DECK_FILE);
            break;
        case(BAD_START):
            fprintf(stderr, "Bad start\n");
            exit(BAD_START);
            break;
        case(CLIENT_DISCONNECT):
            fprintf(stderr, "Client disconnected\n");
            exit(CLIENT_DISCONNECT);
            break;
        case(PROTOCOL_ERR):
            fprintf(stderr, "Protocol error by client\n");
            exit(PROTOCOL_ERR);
            break;
        case(SIGINT_RECIEVED):
            fprintf(stderr, "SIGINT caught\n");
            exit(SIGINT_RECIEVED);
            break;
        default:
            exit(NORMAL);
            break;
    }
}

void check_card(char *content) {
    if (content[0] != 'B' && content[0] != 'Y' && content[0] != 'P' &&
                content[0] != 'R') {
        exit_with_error(INVALID_DECK_FILE);            
    }
    int colAmount = 0, commaAmount = 0;
    for (int i = 1; i < strlen(content); i++) {
        if (content[i] != ':' && content[i] != ',' && content[i] != '\n'
                && !isdigit(content[i])) {
            exit_with_error(INVALID_DECK_FILE);            
        }
        switch(content[i]) {
            case(':'):
                colAmount++;
                break;
            case(','):
                commaAmount++;
                break;
        }
    }       
    if (colAmount != 2 || commaAmount != 3) {
        exit_with_error(INVALID_DECK_FILE);
    }
}

Deck load_deck(char *fileName) {
    FILE *file = fopen(fileName, "r");
    if (file == NULL || !file) {
        exit_with_error(CANNOT_ACCESS_DECK_FILE);
    }
    char *content = malloc(sizeof(char));
    char character;
    int counter = 0, cards = 0;
    while((character = getc(file)) != EOF) {
        content = realloc(content, sizeof(char) * (counter + 1));
        content[counter] = character;
        if (character == '\n') {
            cards++;
        }
        counter++;
    }
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
            card.discount[j] = atoi(commaSplit[j]);
        }
        deck.cards[i] = card;
        free(colSplit);
        free(commaSplit);
    }
    free(content);
    free(cardArr);
    fclose(file);
    return deck;
}

void display_deck(Deck deck) {
    for (int i = 0; i < deck.amount; i++) {
        Card c = deck.cards[i];
        printf("%c %i %i %i %i %i\n", c.colour, c.value, c.discount[0],
                c.discount[1], c.discount[2], c.discount[3]);
    }
}

void free_deck(Deck deck) {
    free(deck.cards);
}

// void setup_players(char *players) {
    
// }

// void setup_player

int main(int argc, char **argv) {
    check_args(argc, argv);
    Deck deck = load_deck("td");
    display_deck(deck);
    free_deck(deck);
    char **players = malloc(0);
    for (int i = 4; i < argc; i++) {
        players = realloc(sizeof(char *) * (i - 3));
        players[i - 4] = argv[i];
    }
    // setup_players();
    // int fd[2];
    // pipe(fd);
    
    // pid_t p = fork();
    // if (p < 0) {
    //     printf("fork failed\n");
    //     exit(1);
    // } else if (p > 0) {
    //     close(fd[READ]);
    //     write(fd[WRITE], "egg", 10);
    //     wait(0);
    //     printf("from parent\n");
    // } else {
        
    //     char *args[1] = {"e"};
    //     execvp("./test", args);
    //     printf("from child, execvp failed\n");
    // }
}
