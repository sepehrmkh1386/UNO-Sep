#ifndef PLAYER_H
#define PLAYER_H

#include <string>

#include "../ds/SinglyLinkedList.h"
#include "Account.h"
#include "Card.h"
#include "RoundState.h"

using namespace std;

class Player {
private:
    Account* account;
    string name;
    SinglyLinkedList<Card> hand;
    SinglyLinkedList<int> roundScores;
    int currentGameScore;

public:
    Player(const string& playerName, Account* playerAccount);
    virtual ~Player() {}

    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;

    virtual bool isHuman() const = 0;
    virtual Action chooseAction(const RoundState& state) = 0;

    const string& getName() const;
    Account* getAccount() const;

    void addCard(const Card& card);
    bool removeCard(int index, Card& removed);
    bool getCard(int index, Card& result) const;
    int getHandSize() const;
    const SinglyLinkedList<Card>& getHand() const;
    void sortHand();
    int handScore() const;
    void clearHand();

    void recordRoundScore(int points);
    int getCurrentGameScore() const;
    const SinglyLinkedList<int>& getRoundScores() const;
    void resetForNewGame();
};

#endif