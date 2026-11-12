#include <iostream>

#include "../src/logic/Deck.h"

using namespace std;

static int failedChecks = 0;

static void check(bool condition, const char* name) {
    if (condition) {
        cout << "[PASS] " << name << '\n';
    } else {
        cout << "[FAIL] " << name << '\n';
        ++failedChecks;
    }
}

static void tally(const Card& card, int colorCount[5], int typeCount[6]) {
    ++colorCount[static_cast<int>(card.getColor())];
    ++typeCount[static_cast<int>(card.getType())];
}

static bool isFullDeckTally(const int colorCount[5], const int typeCount[6]) {
    return colorCount[0] == 25 && colorCount[1] == 25 &&
           colorCount[2] == 25 && colorCount[3] == 25 &&
           colorCount[4] == 8 &&
           typeCount[0] == 76 && typeCount[1] == 8 &&
           typeCount[2] == 8 && typeCount[3] == 8 &&
           typeCount[4] == 4 && typeCount[5] == 4;
}

static void testDiscardPile() {
    DiscardPile pile;
    Card card;

    check(pile.isEmpty() && pile.size() == 0, "discard: starts empty");
    check(!pile.peek(card) && !pile.pop(card),
          "discard: peek and pop on empty fail");

    pile.push(Card(CardColor::Red, 1));
    pile.push(Card(CardColor::Blue, 2));
    check(pile.size() == 2 && pile.peek(card) &&
          card.getColor() == CardColor::Blue && card.getValue() == 2,
          "discard: peek returns the last pushed card");

    bool order = pile.pop(card) && card.getValue() == 2;
    order = order && pile.pop(card) && card.getValue() == 1;
    check(order && pile.isEmpty(), "discard: pop follows LIFO order");

    pile.push(Card(CardColor::Green, 3));
    pile.clear();
    check(pile.isEmpty() && pile.size() == 0, "discard: clear empties the pile");
}

static void testBuildAndDraw() {
    Deck deck;
    DiscardPile discard;

    check(deck.isEmpty() && deck.size() == 0, "deck: starts empty");

    deck.build();
    check(deck.size() == 108, "deck: build creates 108 cards");

    DynamicArray<Card> standard;
    buildStandardDeck(standard);

    int colorCount[5] = {0, 0, 0, 0, 0};
    int typeCount[6] = {0, 0, 0, 0, 0, 0};
    bool allDrawn = true;
    bool sameOrder = true;

    for (int i = 0; i < 108; ++i) {
        Card drawn;
        Card expected;

        if (!deck.drawCard(drawn, discard)) {
            allDrawn = false;
            break;
        }

        standard.get(i, expected);

        if (drawn.toString() != expected.toString()) {
            sameOrder = false;
        }

        tally(drawn, colorCount, typeCount);
    }

    check(allDrawn, "deck: 108 draws all succeed");
    check(isFullDeckTally(colorCount, typeCount),
          "deck: drawn cards are exactly the standard deck");
    check(!sameOrder, "deck: build shuffles the cards");
    check(deck.isEmpty() && discard.isEmpty(),
          "deck: empty after drawing everything");

    Card extra;
    check(!deck.drawCard(extra, discard),
          "deck: draw fails when deck and discard pile are both empty");

    deck.build();
    deck.build();
    check(deck.size() == 108, "deck: building again gives a fresh 108 cards");
}

static void testReshuffle() {
    Deck deck;
    DiscardPile discard;

    Card first(CardColor::Red, 5);

    Card second(CardColor::None, CardType::Wild);
    second.setColor(CardColor::Red);

    Card third(CardColor::Blue, CardType::Skip);

    Card top(CardColor::None, CardType::WildDrawFour);
    top.setColor(CardColor::Green);

    discard.push(first);
    discard.push(second);
    discard.push(third);
    discard.push(top);

    Card drawn[3];
    check(deck.isEmpty() && deck.drawCard(drawn[0], discard),
          "reshuffle: draw from empty deck rebuilds it from the discard pile");

    Card seenTop;
    check(discard.size() == 1 && discard.peek(seenTop) &&
          seenTop.getType() == CardType::WildDrawFour &&
          seenTop.getColor() == CardColor::Green,
          "reshuffle: the top card stays on the table with its color");
    check(deck.size() == 2, "reshuffle: the other 3 cards went to the deck");

    bool secondDraw = deck.drawCard(drawn[1], discard);
    bool thirdDraw = deck.drawCard(drawn[2], discard);
    check(secondDraw && thirdDraw, "reshuffle: remaining cards can be drawn");

    int colorCount[5] = {0, 0, 0, 0, 0};
    int typeCount[6] = {0, 0, 0, 0, 0, 0};
    bool wildColorsReset = true;

    for (int i = 0; i < 3; ++i) {
        tally(drawn[i], colorCount, typeCount);

        if (drawn[i].isWildCard() &&
            drawn[i].getColor() != CardColor::None) {
            wildColorsReset = false;
        }
    }

    check(typeCount[0] == 1 && typeCount[1] == 1 && typeCount[4] == 1,
          "reshuffle: the same three cards came back");
    check(wildColorsReset, "reshuffle: wild colors were reset to None");

    Card more;
    check(!deck.drawCard(more, discard),
          "reshuffle: draw fails when only the top card is left");
    check(discard.size() == 1 && discard.peek(seenTop) &&
          seenTop.getColor() == CardColor::Green,
          "reshuffle: the top card is still untouched after the failed draw");
}

static void testNoCardsLost() {
    Deck deck;
    DiscardPile discard;

    deck.build();

    for (int i = 0; i < 108; ++i) {
        Card card;
        deck.drawCard(card, discard);
        discard.push(card);
    }

    check(deck.isEmpty() && discard.size() == 108,
          "cycle: all 108 cards moved to the discard pile");

    Card topBefore;
    discard.peek(topBefore);

    Card drawn;
    check(deck.drawCard(drawn, discard), "cycle: draw triggers a rebuild");

    Card topAfter;
    check(discard.size() == 1 && discard.peek(topAfter) &&
          topAfter.toString() == topBefore.toString(),
          "cycle: only the previous top card stays on the table");
    check(deck.size() == 106, "cycle: 106 cards in the deck after one draw");
    check(deck.size() + discard.size() + 1 == 108,
          "cycle: no card was lost or duplicated");
}

int main() {
    cout << "--- DiscardPile tests ---\n";
    testDiscardPile();

    cout << "\n--- Deck build and draw tests ---\n";
    testBuildAndDraw();

    cout << "\n--- Reshuffle tests ---\n";
    testReshuffle();

    cout << "\n--- Full cycle test ---\n";
    testNoCardsLost();

    cout << "\nFailed checks: " << failedChecks << '\n';
    return failedChecks == 0 ? 0 : 1;
}