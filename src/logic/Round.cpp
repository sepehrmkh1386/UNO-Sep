#include "Round.h"
#include "Random.h"
#include "Recursion.h"

Round::Round(const DynamicArray<Player*>& seats)
    : pendingPenalty(0),
      activeColor(CardColor::None),
      awaitingStartColor(false),
      roundOver(false),
      winner(nullptr),
      hasDrawn(false),
      lastDrawCount(0) {
    for (int i = 0; i < seats.size(); ++i) {
        Player* player = nullptr;
        seats.get(i, player);
        turns.addPlayer(player);
    }
}

int Round::drawInto(Player* player, int count) {
    int drawn = 0;

    for (int i = 0; i < count; ++i) {
        Card card;

        if (!deck.drawCard(card, discardPile)) {
            break;
        }

        player->addCard(card);
        ++drawn;
    }

    return drawn;
}

void Round::dealCards() {
    for (int i = 0; i < turns.size(); ++i) {
        drawInto(turns.getSeat(i), 7);
    }
}

void Round::placeFirstCard() {
    Card first;
    bool found = false;
    int attempts = 0;

    while (!found && attempts < 1000) {
        ++attempts;

        if (!deck.drawCard(first, discardPile)) {
            return;
        }

        if (first.getType() == CardType::WildDrawFour) {
            deck.returnAndReshuffle(first);
        } else {
            found = true;
        }
    }

    if (!found) {
        return;
    }

    discardPile.push(first);
    Player* current = turns.getCurrentPlayer();

    switch (first.getType()) {
        case CardType::Number:
            activeColor = first.getColor();
            break;
        case CardType::Skip:
            activeColor = first.getColor();
            turns.advanceTurn(1);
            break;
        case CardType::Reverse:
            activeColor = first.getColor();

            if (turns.size() == 2) {
                turns.advanceTurn(1);
            } else {
                turns.reverseDirection();
            }
            break;
        case CardType::DrawTwo:
            activeColor = first.getColor();
            drawInto(current, 2);
            turns.advanceTurn(1);
            break;
        case CardType::Wild:
            activeColor = CardColor::None;
            awaitingStartColor = true;
            break;
        case CardType::WildDrawFour:
            break;
    }
}

void Round::moveTurnAfter(const Card& card) {
    switch (card.getType()) {
        case CardType::Skip:
            turns.advanceTurn(2);
            break;
        case CardType::Reverse:
            if (turns.size() == 2) {
                turns.advanceTurn(2);
            } else {
                turns.reverseDirection();
                turns.advanceTurn(1);
            }
            break;
        case CardType::Number:
        case CardType::DrawTwo:
        case CardType::Wild:
        case CardType::WildDrawFour:
            turns.advanceTurn(1);
            break;
    }
}

void Round::endTurn() {
    Player* player = turns.getCurrentPlayer();

    if (player != nullptr) {
        player->sortHand();
    }

    hasDrawn = false;
    turns.advanceTurn(1);
}

bool Round::topIsPlusFour() const {
    Card top;

    return discardPile.peek(top) && top.getType() == CardType::WildDrawFour;
}

MoveResult Round::executePlay(int cardIndex, CardColor chosenColor) {
    if (roundOver) {
        return MoveResult::RoundOver;
    }

    if (awaitingStartColor) {
        return MoveResult::AwaitingStartColor;
    }

    Player* player = turns.getCurrentPlayer();
    Card card;

    if (player == nullptr || !player->getCard(cardIndex, card)) {
        return MoveResult::InvalidIndex;
    }

    Card topCard;
    discardPile.peek(topCard);

    if (!RuleManager::isValidPlay(card, topCard, activeColor,
                                  player->getHand(), penaltyStack)) {
        return MoveResult::InvalidMove;
    }

    if (card.isWildCard() && chosenColor == CardColor::None) {
        return MoveResult::ColorRequired;
    }

    player->removeCard(cardIndex, card);

    if (card.isWildCard()) {
        card.setColor(chosenColor);
    }

    discardPile.push(card);
    activeColor = card.getColor();

    int penalty = RuleManager::penaltyValue(card);

    if (penalty > 0) {
        penaltyStack.push(penalty);
        pendingPenalty += penalty;
    }

    hasDrawn = false;

    if (player->getHandSize() == 0) {
        roundOver = true;
        winner = player;
        return MoveResult::Ok;
    }

    player->sortHand();
    moveTurnAfter(card);

    return MoveResult::Ok;
}

void Round::start() {
    penaltyStack.clear();
    pendingPenalty = 0;
    activeColor = CardColor::None;
    awaitingStartColor = false;
    roundOver = false;
    winner = nullptr;
    hasDrawn = false;
    lastDrawCount = 0;
    discardPile.clear();

    for (int i = 0; i < turns.size(); ++i) {
        turns.getSeat(i)->clearHand();
    }

    deck.build();
    dealCards();
    placeFirstCard();

    for (int i = 0; i < turns.size(); ++i) {
        turns.getSeat(i)->sortHand();
    }
}

MoveResult Round::playCard(int cardIndex, CardColor chosenColor) {
    if (hasDrawn) {
        return MoveResult::NotAllowed;
    }

    return executePlay(cardIndex, chosenColor);
}

MoveResult Round::draw() {
    if (roundOver) {
        return MoveResult::RoundOver;
    }

    if (awaitingStartColor) {
        return MoveResult::AwaitingStartColor;
    }

    if (hasDrawn) {
        return MoveResult::NotAllowed;
    }

    Player* player = turns.getCurrentPlayer();
    lastDrawCount = 0;

    if (pendingPenalty > 0) {
        int amount = sumPenalty(penaltyStack);
        pendingPenalty = 0;

        lastDrawCount = drawInto(player, amount);
        endTurn();

        return MoveResult::Ok;
    }

    lastDrawCount = drawInto(player, 1);

    if (lastDrawCount == 0) {
        endTurn();
        return MoveResult::Ok;
    }

    Card drawn;
    player->getCard(player->getHandSize() - 1, drawn);

    Card topCard;
    discardPile.peek(topCard);

    if (RuleManager::isValidPlay(drawn, topCard, activeColor,
                                 player->getHand(), penaltyStack)) {
        hasDrawn = true;
    } else {
        endTurn();
    }

    return MoveResult::Ok;
}

MoveResult Round::playDrawn(CardColor chosenColor) {
    if (roundOver) {
        return MoveResult::RoundOver;
    }

    if (!hasDrawn) {
        return MoveResult::NotAllowed;
    }

    Player* player = turns.getCurrentPlayer();

    return executePlay(player->getHandSize() - 1, chosenColor);
}

MoveResult Round::pass() {
    if (roundOver) {
        return MoveResult::RoundOver;
    }

    if (!hasDrawn) {
        return MoveResult::NotAllowed;
    }

    endTurn();

    return MoveResult::Ok;
}

MoveResult Round::chanceWithRoll(int roll) {
    if (roundOver) {
        return MoveResult::RoundOver;
    }

    if (awaitingStartColor) {
        return MoveResult::AwaitingStartColor;
    }

    if (pendingPenalty <= 0) {
        return MoveResult::NotAllowed;
    }

    int cards = chanceMachine.spinWithRoll(topIsPlusFour(), roll);

    if (cards < 0) {
        return MoveResult::InvalidMove;
    }

    Player* player = turns.getCurrentPlayer();

    penaltyStack.clear();
    pendingPenalty = 0;

    lastDrawCount = drawInto(player, cards);
    endTurn();

    return MoveResult::Ok;
}

MoveResult Round::chance() {
    int total = chanceMachine.totalWeight(topIsPlusFour());

    return chanceWithRoll(randomInt(total));
}

MoveResult Round::applyAction(const Action& action) {
    switch (action.type) {
        case ActionType::Play:
            return playCard(action.cardIndex, action.color);
        case ActionType::Draw:
            return draw();
        case ActionType::PlayDrawn:
            return playDrawn(action.color);
        case ActionType::Pass:
            return pass();
        case ActionType::Chance:
            return chance();
        case ActionType::None:
            return MoveResult::NotAllowed;
    }

    return MoveResult::NotAllowed;
}

bool Round::setStartColor(CardColor color) {
    if (!awaitingStartColor || color == CardColor::None) {
        return false;
    }

    Card top;

    if (!discardPile.pop(top)) {
        return false;
    }

    top.setColor(color);
    discardPile.push(top);

    activeColor = color;
    awaitingStartColor = false;

    return true;
}

void Round::setTopCard(const Card& card) {
    discardPile.push(card);
    activeColor = card.getColor();
    awaitingStartColor = false;
}

void Round::leaveOneCardInDeck() {
    Card card;

    while (deck.size() > 1) {
        deck.drawCard(card, discardPile);
    }
}

Player* Round::getCurrentPlayer() const {
    return turns.getCurrentPlayer();
}

Player* Round::getPlayerAt(int seat) const {
    return turns.getSeat(seat);
}

int Round::getPlayerCount() const {
    return turns.size();
}

bool Round::getTopCard(Card& result) const {
    return discardPile.peek(result);
}

CardColor Round::getActiveColor() const {
    return activeColor;
}

int Round::getPendingPenalty() const {
    return pendingPenalty;
}

int Round::getDeckSize() const {
    return deck.size();
}

int Round::getDiscardSize() const {
    return discardPile.size();
}

bool Round::isClockwise() const {
    return turns.isClockwiseDirection();
}

bool Round::needsStartColor() const {
    return awaitingStartColor;
}

bool Round::hasDrawnCard() const {
    return hasDrawn;
}

int Round::getLastDrawCount() const {
    return lastDrawCount;
}

bool Round::isOver() const {
    return roundOver;
}

Player* Round::getWinner() const {
    return winner;
}

RoundState Round::getState() const {
    RoundState state;

    discardPile.peek(state.topCard);
    state.activeColor = activeColor;
    state.pendingPenalty = pendingPenalty;
    state.hasDrawn = hasDrawn;

    return state;
}