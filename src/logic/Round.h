#ifndef ROUND_H
#define ROUND_H

#include "../ds/CustomStack.h"
#include "../ds/DynamicArray.h"
#include "../model/Card.h"
#include "../model/Player.h"
#include "../model/RoundState.h"
#include "Deck.h"
#include "RuleManager.h"
#include "TurnManager.h"
#include "WeightedRandom.h"

enum class MoveResult {
    Ok,
    InvalidIndex,
    InvalidMove,
    ColorRequired,
    AwaitingStartColor,
    RoundOver,
    NotAllowed
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
    bool hasDrawn;
    int lastDrawCount;
    ChanceMachine chanceMachine;

    int drawInto(Player* player, int count);
    void dealCards();
    void placeFirstCard();
    void moveTurnAfter(const Card& card);
    void endTurn();
    bool topIsPlusFour() const;
    MoveResult executePlay(int cardIndex, CardColor chosenColor);

public:
    Round(const DynamicArray<Player*>& seats);

    Round(const Round&) = delete;
    Round& operator=(const Round&) = delete;

    void start();

    MoveResult playCard(int cardIndex, CardColor chosenColor);
    MoveResult draw();
    MoveResult playDrawn(CardColor chosenColor);
    MoveResult pass();
    MoveResult chance();
    MoveResult chanceWithRoll(int roll);
    MoveResult applyAction(const Action& action);

    bool setStartColor(CardColor color);
    void setTopCard(const Card& card);
    void leaveOneCardInDeck();

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
    bool hasDrawnCard() const;
    int getLastDrawCount() const;
    bool isOver() const;
    Player* getWinner() const;
    RoundState getState() const;
};

#endif