#include "player.h"
#define PLAYER_NAME "shenzi"

int main(int argc, char **argv) {
    // fprintf(stderr, "argc: %i, argv0: %s, argv1: %s\n", argc, argv[0], argv[1]);
    // check_args(argc, argv, PLAYER_NAME);
    play_game("2", argv[PLAYER_ID], PLAYER_NAME);
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
                //fprintf(stderr, "wildused: %i, player %i wild: %i\n", wildUsed, player->id, player->wildTokens);
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

void cost_of_card(Card card, Player *player, int finalCost[5]) { //Tp,Tb,Ty,Tr,Tw;
    for (int i = 0; i < 5; i++) {
        finalCost[i] = 0;
    }
    for (int i = 0; i < 4; i++) {
        //fprintf(stderr, "card costs %i, player has %i tokens %i wild\n", card.cost[0], player->tokens[0], player->wildTokens);
        int priceAfterDiscount = card.cost[i] - player->currentDiscount[i];
        if (priceAfterDiscount > player->tokens[i]) {
            finalCost[WILD] += priceAfterDiscount - player->tokens[i];
            finalCost[i] += player->tokens[i];
        } else {
            finalCost[i] += priceAfterDiscount;
        }
        if (finalCost[i] < 0) {
            finalCost[i] = 0;
        }
        //fprintf(stderr, "price after discount = %i, using %i wild\n", priceAfterDiscount, finalCost[4]);
    }
}

void f(Player *player) {
    // fprintf(stderr, "Player %i, tokens: %i %i %i %i %i\ndiscount: %i %i %i %i\n", player->id, player->tokens[0],
    // player->tokens[1],player->tokens[2],player->tokens[3],player->wildTokens
    // ,player->currentDiscount[0],player->currentDiscount[1],player->currentDiscount[2],player->currentDiscount[3]);
}

int attempt_purchase(Game *game, Player *player) {
    // fprintf(stderr, "Player 0 ===\n");
    // fprintf(stderr, "player %c has %i wild\n", player->id, player->wildTokens);
    Deck deck = affordable_cards(game->deckFaceup, player);
    if (deck.amount == 0) {
        free(deck.cards);
        return 0;
    }
    //fprintf(stderr, "can afford %i cards\n", deck.amount);
    //display_deck(deck);
    Deck d = get_card_by_value(deck, largest_value(deck));
    //fprintf(stderr, "potenital cards: \n");
    //display_deck(d);
    //fprintf(stderr, "before cost card player %i tokens: %i %i %i %i\n", player->id, player->tokens[0], player->tokens[1], player->tokens[2], player->tokens[3]);
    int cost[5];
    cost_of_card(d.cards[0], player, cost);
    int index = index_of_card(game->deckFaceup, d.cards[0], TRUE);
    //fprintf(stderr, "cost:%i %i %i %i %i", cost[0], cost[1], cost[2], cost[3], cost[4]);
    // fprintf(stderr, "%i player has %i\n", player->id, player->tokens[0]);
    // display_deck(game->deckFaceup);
    // fprintf(stderr, "%i PURCHASING %i %i %i %i\n", player->id, d.cards[0].cost[0], d.cards[0].cost[1], d.cards[0].cost[2] ,d.cards[0].cost[3]);
    // f(player);
    send_message("purchase%i:%i,%i,%i,%i,%i\n", index, cost[PURPLE],
            cost[BLUE], cost[YELLOW], cost[RED], cost[WILD]);
    free(d.cards);
    free(deck.cards);
    return 1;
}

int can_take_tokens(Game *game, Player *player) {
    int emptyPile = 0;
    for (int i = 0; i < 4; i++) {
        // fprintf(stderr, "tokenpile[%i] = %i : %i\n", i, game->tokenPile[i], player->tokens[i]);
        if (game->tokenPile[i] == 0) {
            emptyPile++;
        }
    }
    if (emptyPile > 1) {
        // fprintf(stderr, "empty piles\n");
        return 0;
    }
    return 1;
}

int attempt_take_tokens(Game *game, Player *player) {
    if (!can_take_tokens(game, player)) {
        // fprintf(stderr, "cannot take\n");
        return 0;
    }
    int tokenTakenTotal = 0;
    int tokenPurple = 0, tokenBlue = 0, tokenYellow = 0, tokenRed = 0;
    for (int i = 0; i < 4; i++) {
        if (tokenTakenTotal > 2) {
            break;
        }
        if (game->tokenPile[i] > 0) {
            // player->tokens[i]++;
            // game->tokenPile[i]--;
            tokenTakenTotal++;
            switch (i) {
                case (PURPLE):
                    tokenPurple = 1;
                    break;
                case (BLUE):
                    tokenBlue = 1;
                    break;
                case (YELLOW):
                    tokenYellow = 1;
                    break;
                case (RED):
                    tokenRed = 1;
                    break;
            }
        }
    }
    send_message("take%i,%i,%i,%i\n",
            tokenPurple, tokenBlue, tokenYellow, tokenRed);
    return 1;
}

void process_dowhat(Game *game, Player *player) {
    fprintf(stderr, "Received dowhat\n");
    // fprintf(stderr, "---processing dowhat by %s\n", PLAYER_NAME);
    // send_message("purchase1:0,0,0,0,0");
    // if (player->id == 0) {
    if (attempt_purchase(game, player)) {
        // display_deck(game->deckFaceup);
        // fprintf(stderr, "%i, purchasing card\n", player->id);
        // send_message("wild\n");
    } else if (attempt_take_tokens(game, player)) {
        // fprintf(stderr, "%i, taking tokens\n", player->id);
    } else {
        // fprintf(stderr, "%i, taking wild\n", player->id);
        send_message("wild\n");
    }
}