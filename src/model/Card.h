#ifndef CARD_H
#define CARD_H

#include <string>

#include "../ds/DynamicArray.h"

using namespace std;

enum class CardColor {
    Red,
    Green,
    Blue,
    Yellow,
    None
};

enum class CardType {
    Number,
    Skip,
    Reverse,
    DrawTwo,
    Wild,
    WildDrawFour
};

inline string colorToString(CardColor color) {
    switch (color) {
        case CardColor::Red:
            return "RED";
        case CardColor::Green:
            return "GREEN";
        case CardColor::Blue:
            return "BLUE";
        case CardColor::Yellow:
            return "YELLOW";
        case CardColor::None:
            return "NONE";
    }

    return "NONE";
}

class Card {
private:
    CardColor color;
    CardType type;
    int value;

    static int defaultValueOf(CardType cardType) {
        switch (cardType) {
            case CardType::Skip:
            case CardType::Reverse:
            case CardType::DrawTwo:
                return 20;
            case CardType::Wild:
            case CardType::WildDrawFour:
                return 50;
            default:
                return 0;
        }
    }

public:
    Card()
        : color(CardColor::None), type(CardType::Number), value(0) {
    }

    Card(CardColor cardColor, int number)
        : color(cardColor), type(CardType::Number), value(number) {
    }

    Card(CardColor cardColor, CardType cardType)
        : color(cardColor), type(cardType), value(defaultValueOf(cardType)) {
    }

    CardColor getColor() const {
        return color;
    }

    CardType getType() const {
        return type;
    }

    int getValue() const {
        return value;
    }

    void setColor(CardColor newColor) {
        if (type == CardType::Wild ||
            type == CardType::WildDrawFour) {
            if (newColor != CardColor::None) {
                color = newColor;
            }
        }
    }

    void resetColor() {
        if (type == CardType::Wild ||
            type == CardType::WildDrawFour) {
            color = CardColor::None;
        }
    }

    bool isNumberCard() const {
        return type == CardType::Number;
    }

    bool isWildCard() const {
        return type == CardType::Wild ||
               type == CardType::WildDrawFour;
    }

    string toString() const {
        switch (type) {
            case CardType::Number:
                return colorToString(color) + " " + to_string(value);
            case CardType::Skip:
                return colorToString(color) + " SKIP";
            case CardType::Reverse:
                return colorToString(color) + " REVERSE";
            case CardType::DrawTwo:
                return colorToString(color) + " DRAW TWO (+2)";
            case CardType::Wild:
                return "WILD";
            case CardType::WildDrawFour:
                return "WILD DRAW FOUR (+4)";
        }

        return "UNKNOWN";
    }
};

inline void buildStandardDeck(DynamicArray<Card>& cards) {
    const CardColor colors[4] = {
        CardColor::Red,
        CardColor::Green,
        CardColor::Blue,
        CardColor::Yellow
    };

    for (int c = 0; c < 4; ++c) {
        cards.add(Card(colors[c], 0));

        for (int number = 1; number <= 9; ++number) {
            cards.add(Card(colors[c], number));
            cards.add(Card(colors[c], number));
        }

        for (int copy = 0; copy < 2; ++copy) {
            cards.add(Card(colors[c], CardType::Skip));
            cards.add(Card(colors[c], CardType::Reverse));
            cards.add(Card(colors[c], CardType::DrawTwo));
        }
    }

    for (int copy = 0; copy < 4; ++copy) {
        cards.add(Card(CardColor::None, CardType::Wild));
        cards.add(Card(CardColor::None, CardType::WildDrawFour));
    }
}

#endif