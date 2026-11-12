#ifndef DECK_H
#define DECK_H

#include "../ds/CustomQueue.h"
#include "../ds/CustomStack.h"
#include "../ds/DynamicArray.h"
#include "../model/Card.h"

class DiscardPile {
private:
    CustomStack<Card> playedStack;

public:
    bool isEmpty() const;
    int size() const;
    void push(const Card& card);
    bool pop(Card& result);
    bool peek(Card& result) const;
    void clear();
};

class Deck {
private:
    CustomQueue<Card> drawQueue;

    static void shuffleCards(DynamicArray<Card>& cards);
    void enqueueAll(const DynamicArray<Card>& cards);
    void rebuildFrom(DiscardPile& discardPile);

public:
    void build();
    bool isEmpty() const;
    int size() const;
    bool drawCard(Card& result, DiscardPile& discardPile);
    void clear();
};

#endif