#include "player.h"
#define PLAYER_NAME "shenzi"

/**
 * Function Prototypes
 **/
int attempt_purchase(Game *game, Player *player);
int attempt_take_tokens(Game *game, Player *player);

/** Main */
int main(int argc, char **argv) {
    check_args(argc, argv, PLAYER_NAME);
    play_game(argv[TOTAL_PLAYERS], argv[PLAYER_ID], PLAYER_NAME);
    return 0;
}

/**
* Attempt to purchase an card by player shenzi
* @param game - The game instance.
* @param player - The active player.
* @return int - 1 if can and have purchased an card.
*/
int attempt_purchase(Game *game, Player *player) {
    Deck deck = affordable_cards(game->deckFaceup, player);
    if (deck.amount == 0) {
        free(deck.cards);
        return 0;
    }
    Deck d = get_card_by_value(deck, largest_value(deck));
    int cheapestCost = 0;
    for (int i = 0; i < d.amount; i++) {
        int cost[5];
        cost_of_card(d.cards[i], player, cost);
        int sum = sum_cost(cost);
        if (cheapestCost == 0 || sum < cheapestCost) {
            cheapestCost = sum;
        }
    }
    Deck cheapest = get_card_by_cost(d, player, cheapestCost);
    int index = 0;
    for (int i = 0; i < cheapest.amount; i++) {
        int cardIndex = index_of_card(game->deckFaceup,
                cheapest.cards[i], TRUE);
        if (cardIndex > index) {
            index = cardIndex;
        }
    }
    purchase_card(game, player, index);
    free(d.cards);
    free(deck.cards);
    free(cheapest.cards);
    return 1;
}

/**
* Attempt to take tokens.
* @param game - The game instance.
* @param player - The active player.
* @return int - 1 if tokens can be and has been taken.
*/
int attempt_take_tokens(Game *game, Player *player) {
    if (!can_take_tokens(game, player)) {
        return 0;
    }
    int tokenTakenTotal = 0;
    int tokenPurple = 0, tokenBROWN = 0, tokenYellow = 0, tokenRed = 0;
    for (int i = 0; i < 4; i++) {
        if (tokenTakenTotal > 2) {
            break;
        }
        if (game->tokenPile[i] > 0) {
            tokenTakenTotal++;
            switch (i) {
                case (PURPLE):
                    tokenPurple = 1;
                    break;
                case (BROWN):
                    tokenBROWN = 1;
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
            tokenPurple, tokenBROWN, tokenYellow, tokenRed);
    return 1;
}

/**
* Processes downhat by shenzi.
* @param game - The game instance.
* @param player - The active player.
*/
void process_dowhat(Game *game, Player *player) {
    if (attempt_purchase(game, player)) {
    } else if (attempt_take_tokens(game, player)) {
    } else {
        send_message("wild\n");
    }
}