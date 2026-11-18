#ifndef ROUND_H
#define ROUND_H

#include "../ds/CustomStack.h"
#include "../ds/DynamicArray.h"
#include "../model/Card.h"
#include "../model/Player.h"
#include "Deck.h"
#include "RuleManager.h"
#include "TurnManager.h"

enum class MoveResult {
    Ok,
    InvalidIndex,
    InvalidMove,
    ColorRequired,
    AwaitingStartColor,
    RoundOver
};

class Round {
private:
    TurnManager turns;
    Deck deck;
    DiscardPile discardPile;
    CustomStack<int> penaltyStack;
    int pendingPenalty;
    CardColor activeColor;
    bool awaitingStartColor;
    bool roundOver;
    Player* winner;

    int drawInto(Player* player, int count);
    void dealCards();
    void placeFirstCard();
    void moveTurnAfter(const Card& card);

public:
    Round(const DynamicArray<Player*>& seats);

    Round(const Round&) = delete;
    Round& operator=(const Round&) = delete;

    void start();

    MoveResult playCard(int cardIndex, CardColor chosenColor);
    bool setStartColor(CardColor color);
    void setTopCard(const Card& card);

    Player* getCurrentPlayer() const;
    Player* getPlayerAt(int seat) const;
    int getPlayerCount() const;
    bool getTopCard(Card& result) const;
    CardColor getActiveColor() const;
    int getPendingPenalty() const;
    int getDeckSize() const;
    int getDiscardSize() const;
    bool isClockwise() const;
    bool needsStartColor() const;
    bool isOver() const;
    Player* getWinner() const;
};

#endif