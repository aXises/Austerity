#include "player.h"
#define PLAYER_NAME "ed"

int main(int argc, char **argv) {
    check_args(argc, argv, PLAYER_NAME);
    play_game(argv[TOTAL_PLAYERS], argv[PLAYER_ID], PLAYER_NAME);
    return 0;
}

int attempt_take_tokens(Game *game, Player *player) {
    if (!can_take_tokens(game, player)) {
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
            if (tokenYellow == 0 && game->tokenPile[YELLOW] != 0) {
                tokenYellow = 1;
                continue;
            } else if (tokenRed == 0 && game->tokenPile[RED] != 0) {
                tokenRed = 1;
                continue;
            } else if (tokenBrown == 0 && game->tokenPile[BROWN] != 0) {
                tokenBrown = 1;
                continue;
            } else if (tokenPurple == 0 && game->tokenPile[PURPLE] != 0) {
                tokenPurple = 1;
                continue;
            }
        }
    }
    send_message("take%i,%i,%i,%i\n",
            tokenPurple, tokenBrown, tokenYellow, tokenRed);
    return 1;
}

Deck others_can_afford(Game *game, Player *player) {
    Deck newDeck;
    newDeck.cards = malloc(0);
    int counter = 0;
    for (int i = 0; i < game->playerAmount; i++) {
        if (i == player->id) {
            continue;
        }
        for (int j = 0; j < game->deckFaceup.amount; j++) {
            if (can_afford(game->deckFaceup.cards[j], &game->players[i])) {
                newDeck.cards = realloc(newDeck.cards,
                        sizeof(Card) * (counter + 1));
                newDeck.cards[counter] = game->deckFaceup.cards[j];
                counter++;
            }
        }
    }
    newDeck.amount = counter;
    return newDeck;
}

int ed_attempt_take_tokens(Game *game, Player *player, Card potentialCard) {
    if (!can_take_tokens(game, player)) {
        return 0;
    }
    int tokenPurple = 0, tokenBrown = 0, tokenYellow = 0, tokenRed = 0;
    for (int i = 0; i < 4; i++) {
        if (tokenPurple + tokenBrown + tokenYellow + tokenRed > 2) {
            break;
        }
        if (potentialCard.cost[YELLOW] - player->tokens[YELLOW] > 0 &&
                game->tokenPile[YELLOW] != 0 && tokenYellow == 0) {
            tokenYellow = 1;
            continue;
        } else if (potentialCard.cost[RED] - player->tokens[RED] > 0 &&
                game->tokenPile[RED] != 0 && tokenRed == 0) {
            tokenRed = 1;
            continue;
        } else if (potentialCard.cost[BROWN] - player->tokens[BROWN] > 0 &&
                game->tokenPile[BROWN] != 0 && tokenBrown == 0) {
            tokenBrown = 1;
            continue;
        } else if (tokenPurple == 0 && potentialCard.cost[PURPLE] - 
                player->tokens[PURPLE] > 0 && game->tokenPile[PURPLE] != 0) {
            tokenPurple = 1;
            continue;
        } else {
            if (tokenYellow == 0 && game->tokenPile[YELLOW] != 0) {
                tokenYellow = 1;
            } else if (tokenRed == 0 && game->tokenPile[RED] != 0) {
                tokenRed = 1;
            } else if (tokenBrown == 0 && game->tokenPile[BROWN] != 0) {
                tokenBrown = 1;
            } else if (tokenPurple == 0 && game->tokenPile[PURPLE] != 0) {
                tokenPurple = 1;
            }
        }
        
    }
    send_message("take%i,%i,%i,%i\n",
            tokenPurple, tokenBrown, tokenYellow, tokenRed);
    return 1;
}

Deck next_player_can_afford(Game *game, Player *player, Deck deck) {
    Deck newDeck;
    newDeck.cards = malloc(0);
    int counter = 0;
    for (int i = player->id + 1; i < game->playerAmount; i++) {
        Player next = game->players[i];
        for (int j = 0; j < deck.amount; j++) {
            if (can_afford(deck.cards[j], &next)) {
                newDeck.cards = realloc(newDeck.cards,
                        sizeof(Card) * (counter + 1));
                newDeck.cards[counter] = deck.cards[j];
                counter++;
            }
        }
        if (counter > 0) {
            break;
        }
    }
    if (counter > 0) {
        newDeck.amount = counter;
        return newDeck;
    }
    for (int i = 0; i < player->id; i++) {
        Player before = game->players[i];
        for (int j = 0; j < deck.amount; j++) {
            if (can_afford(deck.cards[j], &before)) {
                newDeck.cards = realloc(newDeck.cards,
                        sizeof(Card) * (counter + 1));
                newDeck.cards[counter] = deck.cards[j];
                counter++;
            }
        }
        if (counter > 0) {
            break;
        }
    }
    newDeck.amount = counter;
    return newDeck;
}

void process_dowhat(Game *game, Player *player) {
    Deck affordableCardsByOther = others_can_afford(game, player);
    int highestValue = largest_value(affordableCardsByOther);
    Deck highestValueDeck = get_card_by_value(affordableCardsByOther,
            highestValue);
    Deck nextPlayerCanAfford = next_player_can_afford(game, player,
            highestValueDeck);
    if (nextPlayerCanAfford.amount == 0) {
        if(!attempt_take_tokens(game, player)) {
            send_message("wild\n");
        }
    } else {
        Deck affordableCards = affordable_cards(nextPlayerCanAfford, player);
        int index = 8;
        for (int i = 0; i < affordableCards.amount; i++) {
            int cardIndex = index_of_card(game->deckFaceup,
                    affordableCards.cards[i], TRUE);
            if (cardIndex < index) {
                index = cardIndex;
            }
        }
        if (index != 8) { // Cannot afford any cards.
            purchase_card(game, player, index);
        } else {
            int index = 8;
            for (int i = 0; i < nextPlayerCanAfford.amount; i++) {
                int cardIndex = index_of_card(game->deckFaceup,
                        nextPlayerCanAfford.cards[i], TRUE);
                if (cardIndex < index) {
                    index = cardIndex;
                }
            }
            if(!ed_attempt_take_tokens(game, player, game->deckFaceup.cards[index])) {
                send_message("wild\n");
            }
        }
        free(affordableCards.cards);
    }
    free(affordableCardsByOther.cards);
    free(highestValueDeck.cards);
    free(nextPlayerCanAfford.cards);
}