#include "player.h"
#define PLAYER_NAME "banzai"

int main(int argc, char **argv) {
    check_args(argc, argv, PLAYER_NAME);
    play_game(argv[TOTAL_PLAYERS], argv[PLAYER_ID], PLAYER_NAME);
    return 0;
}

int get_highest_cost(Deck deck, Player *player) {
    int highestCost = 0;
    for (int i = 0; i < deck.amount; i++) {
        int cost[5];
        cost_of_card(deck.cards[i], player, cost);
        int sum = sum_cost(cost);
        if (sum > highestCost) {
            highestCost = sum;
        }
    }
    return highestCost;
}

int most_wild_tokens(Deck deck, Player *player) {
    int wildTokens = 0;
    for (int i = 0; i < deck.amount; i++) {
        int cost[5];
        cost_of_card(deck.cards[i], player, cost);
        if (cost[WILD] > wildTokens) {
            wildTokens = cost[WILD]; 
        }
    }
    return wildTokens;
}

Deck get_card_by_Wild(Deck deck, Player *player, int wild) {
    Deck newDeck;
    newDeck.cards = malloc(0);
    int counter = 0;
    for (int i = 0; i < deck.amount; i++) {
        int cost[5];
        cost_of_card(deck.cards[i], player, cost);
        if (cost[WILD] == wild) {
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
    Deck deck = affordable_cards(game->deckFaceup, player);
    if (deck.amount == 0) {
        free(deck.cards);
        return 0;
    }
    if (largest_value(deck) == 0) {
        free(deck.cards);
        return 0;
    }
    int highestCost = get_highest_cost(deck, player);
    Deck highest = get_card_by_cost(deck, player, highestCost);
    int mostWildTokens = most_wild_tokens(highest, player);
    Deck mostWildTokensDeck = get_card_by_Wild(highest, player, mostWildTokens);
    int index = 8;
    for (int i = 0; i < mostWildTokensDeck.amount; i++) {
        int cardIndex = index_of_card(game->deckFaceup,
                mostWildTokensDeck.cards[i], FALSE);
        if (cardIndex < index) {
            index = cardIndex;
        }
    }
    int cost[5];
    cost_of_card(game->deckFaceup.cards[index], player, cost);
    send_message("purchase%i:%i,%i,%i,%i,%i\n", index, cost[PURPLE],
            cost[BROWN], cost[YELLOW], cost[RED], cost[WILD]);
    free(deck.cards);
    free(highest.cards);
    free(mostWildTokensDeck.cards);
    return 1;
}

int sum_tokens(Player *player) {
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += player->tokens[i];
    }
    return sum += player->wildTokens;
}

int attempt_take_tokens(Game *game, Player *player) {
    if (!can_take_tokens(game, player) || sum_tokens(player) >= 3) {
        return 0;
    }
    int tokenTakenTotal = 0;
    int tokenPurple = 0, tokenBrown = 0, tokenYellow = 0, tokenRed = 0;
    for (int i = 0; i < 4; i++) {
        if (tokenTakenTotal > 2) {
            break;
        }
        if (game->tokenPile[i] > 0) {
            tokenTakenTotal++;
            if (tokenYellow != 1 && game->tokenPile[YELLOW] != 0) {
                tokenYellow = 1;
                continue;
            } else if (tokenBrown != 1 && game->tokenPile[BROWN] != 0) {
                tokenBrown = 1;
                continue;
            } else if (tokenPurple != 1 && game->tokenPile[PURPLE] != 0) {
                tokenPurple = 1;
                continue;
            } else if (tokenRed != 1 && game->tokenPile[RED] != 0) {
                tokenRed = 1;
                continue;
            }
        }
    }
    send_message("take%i,%i,%i,%i\n",
            tokenPurple, tokenBrown, tokenYellow, tokenRed);
    return 1;
}

void process_dowhat(Game *game, Player *player) {
    if (attempt_take_tokens(game, player)) {
    } else if (attempt_purchase(game, player)) {
    } else {
        send_message("wild\n");
    }
}