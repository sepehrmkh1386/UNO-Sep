#include "Deck.h"
#include "Random.h"

bool DiscardPile::isEmpty() const {
    return playedStack.isEmpty();
}

int DiscardPile::size() const {
    return playedStack.size();
}

void DiscardPile::push(const Card& card) {
    playedStack.push(card);
}

bool DiscardPile::pop(Card& result) {
    return playedStack.pop(result);
}

bool DiscardPile::peek(Card& result) const {
    return playedStack.peek(result);
}

void DiscardPile::clear() {
    playedStack.clear();
}

void Deck::shuffleCards(DynamicArray<Card>& cards) {
    for (int i = cards.size() - 1; i > 0; --i) {
        int j = randomInt(i + 1);
        cards.swap(i, j);
    }
}

void Deck::enqueueAll(const DynamicArray<Card>& cards) {
    for (int i = 0; i < cards.size(); ++i) {
        Card card;
        cards.get(i, card);
        drawQueue.enqueue(card);
    }
}

void Deck::rebuildFrom(DiscardPile& discardPile) {
    Card topCard;

    if (!discardPile.pop(topCard)) {
        return;
    }

    DynamicArray<Card> cards;
    Card card;

    while (discardPile.pop(card)) {
        card.resetColor();
        cards.add(card);
    }

    shuffleCards(cards);
    enqueueAll(cards);

    discardPile.push(topCard);
}

void Deck::build() {
    drawQueue.clear();

    DynamicArray<Card> cards;
    buildStandardDeck(cards);

    shuffleCards(cards);
    enqueueAll(cards);
}

bool Deck::isEmpty() const {
    return drawQueue.isEmpty();
}

int Deck::size() const {
    return drawQueue.size();
}

bool Deck::drawCard(Card& result, DiscardPile& discardPile) {
    if (drawQueue.isEmpty()) {
        rebuildFrom(discardPile);
    }

    return drawQueue.dequeue(result);
}

void Deck::returnAndReshuffle(const Card& card) {
    DynamicArray<Card> cards;
    Card current;

    while (drawQueue.dequeue(current)) {
        cards.add(current);
    }

    cards.add(card);

    shuffleCards(cards);
    enqueueAll(cards);
}

void Deck::clear() {
    drawQueue.clear();
}