#include <iostream>
#include <string>

#include "../src/model/Card.h"

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

static int colorIndex(CardColor color) {
    switch (color) {
        case CardColor::Red:
            return 0;
        case CardColor::Green:
            return 1;
        case CardColor::Blue:
            return 2;
        case CardColor::Yellow:
            return 3;
        default:
            return -1;
    }
}

static void testCardBasics() {
    Card empty;
    check(empty.getColor() == CardColor::None &&
          empty.getType() == CardType::Number &&
          empty.getValue() == 0,
          "card: default constructor");

    Card seven(CardColor::Red, 7);
    check(seven.isNumberCard() && !seven.isWildCard() &&
          seven.getValue() == 7 && seven.getColor() == CardColor::Red,
          "card: number card fields");

    check(Card(CardColor::Blue, CardType::Skip).getValue() == 20 &&
          Card(CardColor::Blue, CardType::Reverse).getValue() == 20 &&
          Card(CardColor::Blue, CardType::DrawTwo).getValue() == 20,
          "card: action cards are worth 20");

    check(Card(CardColor::None, CardType::Wild).getValue() == 50 &&
          Card(CardColor::None, CardType::WildDrawFour).getValue() == 50,
          "card: wild cards are worth 50");

    Card wild(CardColor::None, CardType::Wild);
    check(wild.isWildCard() && !wild.isNumberCard(),
          "card: isWildCard / isNumberCard");

    wild.setColor(CardColor::Blue);
    check(wild.getColor() == CardColor::Blue,
          "card: setColor works on wild");

    wild.setColor(CardColor::None);
    check(wild.getColor() == CardColor::Blue,
          "card: setColor(None) is ignored");

    wild.resetColor();
    check(wild.getColor() == CardColor::None,
          "card: resetColor sets wild color back to None");

    seven.setColor(CardColor::Green);
    seven.resetColor();
    check(seven.getColor() == CardColor::Red,
          "card: setColor and resetColor do not affect non-wild cards");
}

static void testCardStrings() {
    check(Card(CardColor::Red, 7).toString() == "RED 7",
          "toString: number card");
    check(Card(CardColor::Green, CardType::Skip).toString() == "GREEN SKIP",
          "toString: skip");
    check(Card(CardColor::Blue, CardType::Reverse).toString() == "BLUE REVERSE",
          "toString: reverse");
    check(Card(CardColor::Yellow, CardType::DrawTwo).toString() ==
          "YELLOW DRAW TWO (+2)",
          "toString: draw two");
    check(Card(CardColor::None, CardType::Wild).toString() == "WILD",
          "toString: wild");
    check(Card(CardColor::None, CardType::WildDrawFour).toString() ==
          "WILD DRAW FOUR (+4)",
          "toString: wild draw four");
    check(colorToString(CardColor::Red) == "RED" &&
          colorToString(CardColor::Green) == "GREEN" &&
          colorToString(CardColor::Blue) == "BLUE" &&
          colorToString(CardColor::Yellow) == "YELLOW",
          "colorToString: four colors");
}

static void testStandardDeck() {
    DynamicArray<Card> deck;
    buildStandardDeck(deck);

    check(deck.size() == 108, "deck: has 108 cards");

    int zeros = 0;
    int numbers1to9 = 0;
    int skips = 0;
    int reverses = 0;
    int drawTwos = 0;
    int wilds = 0;
    int wildDrawFours = 0;
    int perColor[4] = {0, 0, 0, 0};
    int numberCount[4][10] = {};
    bool wildsHaveNoColor = true;
    bool valuesCorrect = true;

    for (int i = 0; i < deck.size(); ++i) {
        Card card;
        deck.get(i, card);

        int index = colorIndex(card.getColor());

        if (index >= 0) {
            ++perColor[index];
        }

        switch (card.getType()) {
            case CardType::Number:
                if (card.getValue() == 0) {
                    ++zeros;
                } else {
                    ++numbers1to9;
                }

                if (index >= 0 && card.getValue() >= 0 &&
                    card.getValue() <= 9) {
                    ++numberCount[index][card.getValue()];
                } else {
                    valuesCorrect = false;
                }
                break;
            case CardType::Skip:
                ++skips;
                valuesCorrect = valuesCorrect && card.getValue() == 20;
                break;
            case CardType::Reverse:
                ++reverses;
                valuesCorrect = valuesCorrect && card.getValue() == 20;
                break;
            case CardType::DrawTwo:
                ++drawTwos;
                valuesCorrect = valuesCorrect && card.getValue() == 20;
                break;
            case CardType::Wild:
                ++wilds;
                valuesCorrect = valuesCorrect && card.getValue() == 50;
                wildsHaveNoColor = wildsHaveNoColor &&
                                   card.getColor() == CardColor::None;
                break;
            case CardType::WildDrawFour:
                ++wildDrawFours;
                valuesCorrect = valuesCorrect && card.getValue() == 50;
                wildsHaveNoColor = wildsHaveNoColor &&
                                   card.getColor() == CardColor::None;
                break;
        }
    }

    check(zeros == 4, "deck: 4 zero cards");
    check(numbers1to9 == 72, "deck: 72 cards numbered 1 to 9");
    check(skips == 8, "deck: 8 skip cards");
    check(reverses == 8, "deck: 8 reverse cards");
    check(drawTwos == 8, "deck: 8 draw two cards");
    check(wilds == 4, "deck: 4 wild cards");
    check(wildDrawFours == 4, "deck: 4 wild draw four cards");

    check(perColor[0] == 25 && perColor[1] == 25 &&
          perColor[2] == 25 && perColor[3] == 25,
          "deck: 25 colored cards in each color");

    bool distributionOk = true;

    for (int c = 0; c < 4; ++c) {
        for (int n = 0; n <= 9; ++n) {
            int expected = (n == 0) ? 1 : 2;

            if (numberCount[c][n] != expected) {
                distributionOk = false;
            }
        }
    }

    check(distributionOk,
          "deck: each color has one 0 and two of every 1-9");
    check(wildsHaveNoColor, "deck: wild cards start with no color");
    check(valuesCorrect, "deck: every card has the right score value");
}

int main() {
    cout << "--- Card tests ---\n";
    testCardBasics();

    cout << "\n--- toString tests ---\n";
    testCardStrings();

    cout << "\n--- Standard deck tests ---\n";
    testStandardDeck();

    cout << "\nFailed checks: " << failedChecks << '\n';
    return failedChecks == 0 ? 0 : 1;
}