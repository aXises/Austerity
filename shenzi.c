#include "player.h"
#define PLAYER_NAME "shenzi"

int main(int argc, char **argv) {
    check_args(argc, argv, PLAYER_NAME);
    // fprintf(stderr, "argc: %i, argv0: %s, argv1: %s\n", argc, argv[0], argv[1]);
    play_game(argv[1], PLAYER_NAME);
}

int largest_value(Deck deck) {
    int largest = 0;
    for (int i = 0; i < deck.amount; i++) {
        if (deck.cards[i].value > largest) {
            largest = deck.cards[i].value;
        }
    }
    return largest;
}

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

int can_afford(Card card, Player *player) {
    int wildUsed = 0;
    for (int i = 0; i < 4; i++) {
        if (card.cost[i] - player->currentDiscount[i] > player->tokens[i]) {
            wildUsed += card.cost[i] - player->tokens[i];
            if (wildUsed > player->wildTokens) {
                return 0;
            }
        }
    }
    return 1;
}

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

int attempt_purchase(Game *game, Player *player) {
    fprintf(stderr, "Player 0 ===\n");
    Deck deck = affordable_cards(game->deckFaceup, player);
    if (deck.amount == 0) {
        free(deck.cards);
        return 0;
    }
    fprintf(stderr, "can afford %i cards\n", deck.amount);
    display_deck(deck);
    Deck d = get_card_by_value(deck, largest_value(deck));
    fprintf(stderr, "potenital cards: \n");
    display_deck(d);
    free(d.cards);
    free(deck.cards);
    return 1;
}

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

int attempt_take_tokens(Game *game, Player *player) {
    if (!can_take_tokens(game, player)) {
        return 0;
    }
    int tokenTakenTotal = 0;
    for (int i = 0; i < 4; i++) {
        if (tokenTakenTotal > 3) {
            break;
        }
        if (game->tokenPile[i] > 0) {
            player->tokens[i]++;
            game->tokenPile[i]--;
            tokenTakenTotal++;
        }
    }
    return 1;
}

void take_wild(Game *game, Player *player) {
    fprintf(stderr, "taking wild\n");
    send_message("wild\n");
}

void process_dowhat(Game *game, Player *player) {
    fprintf(stderr, "---processing dowhat by %s\n", PLAYER_NAME);
    // send_message("purchase1:0,0,0,0,0");
    // if (player->id == 0) {
        if (!attempt_purchase(game, player)) {
            fprintf(stderr, "%i cannot purchase\n", player->id);
            if (!attempt_take_tokens(game, player)) {
                fprintf(stderr, "%i cannot take tokens\n", player->id);
                take_wild(game, player);
                return;
            } else {
                return;
            }
        } else {
            return;
        }
    // }
}