#include "TurnManager.h"

TurnManager::TurnManager()
    : current(nullptr), isClockwise(true) {
}

void TurnManager::addPlayer(Player* player) {
    players.add(player);

    if (current == nullptr) {
        current = players.getHead();
    }
}

int TurnManager::size() const {
    return players.size();
}

bool TurnManager::isEmpty() const {
    return players.isEmpty();
}

Player* TurnManager::getCurrentPlayer() const {
    if (current == nullptr) {
        return nullptr;
    }

    return current->data;
}

Player* TurnManager::peekNextPlayer() const {
    if (current == nullptr) {
        return nullptr;
    }

    if (isClockwise) {
        return current->next->data;
    }

    return current->prev->data;
}

Player* TurnManager::getSeat(int index) const {
    Player* result = nullptr;
    players.getAt(index, result);
    return result;
}

void TurnManager::advanceTurn(int steps) {
    if (current == nullptr || steps <= 0) {
        return;
    }

    int moves = steps % players.size();

    for (int i = 0; i < moves; ++i) {
        if (isClockwise) {
            current = current->next;
        } else {
            current = current->prev;
        }
    }
}

void TurnManager::reverseDirection() {
    isClockwise = !isClockwise;
}

bool TurnManager::isClockwiseDirection() const {
    return isClockwise;
}