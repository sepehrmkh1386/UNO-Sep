#include "Player.h"
#include "../logic/Recursion.h"

static bool cardComesBefore(const Card& a, const Card& b) {
    int colorA = static_cast<int>(a.getColor());
    int colorB = static_cast<int>(b.getColor());

    if (colorA != colorB) {
        return colorA < colorB;
    }

    int typeA = static_cast<int>(a.getType());
    int typeB = static_cast<int>(b.getType());

    if (typeA != typeB) {
        return typeA < typeB;
    }

    return a.getValue() < b.getValue();
}

Player::Player(const string& playerName, Account* playerAccount)
    : account(playerAccount), name(playerName), currentGameScore(0) {
}

const string& Player::getName() const {
    return name;
}

Account* Player::getAccount() const {
    return account;
}

void Player::addCard(const Card& card) {
    hand.addLast(card);
}

bool Player::removeCard(int index, Card& removed) {
    return hand.removeAt(index, removed);
}

bool Player::getCard(int index, Card& result) const {
    return hand.get(index, result);
}

int Player::getHandSize() const {
    return hand.size();
}

const SinglyLinkedList<Card>& Player::getHand() const {
    return hand;
}

void Player::sortHand() {
    hand.sort(cardComesBefore);
}

int Player::handScore() const {
    return calcHandScore(hand.getHead());
}

void Player::clearHand() {
    hand.clear();
}

void Player::recordRoundScore(int points) {
    roundScores.addLast(points);
    currentGameScore += points;
}

int Player::getCurrentGameScore() const {
    return currentGameScore;
}

const SinglyLinkedList<int>& Player::getRoundScores() const {
    return roundScores;
}

void Player::resetForNewGame() {
    hand.clear();
    roundScores.clear();
    currentGameScore = 0;
}