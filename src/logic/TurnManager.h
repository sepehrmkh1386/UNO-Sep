#ifndef TURN_MANAGER_H
#define TURN_MANAGER_H

#include "../ds/DoublyCircularLinkedList.h"
#include "../model/Player.h"

class TurnManager {
private:
    DoublyCircularLinkedList<Player*> players;
    DoublyNode<Player*>* current;
    bool isClockwise;

public:
    TurnManager();

    void addPlayer(Player* player);
    int size() const;
    bool isEmpty() const;

    Player* getCurrentPlayer() const;
    Player* peekNextPlayer() const;
    Player* getSeat(int index) const;

    void advanceTurn(int steps);
    void reverseDirection();
    bool isClockwiseDirection() const;
};

#endif