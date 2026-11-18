#include "BotPlayer.h"
#include "../ds/CustomStack.h"
#include "../logic/RuleManager.h"

BotPlayer::BotPlayer(const string& playerName)
    : Player(playerName, nullptr) {
}

bool BotPlayer::isHuman() const {
    return false;
}

CardColor BotPlayer::chooseBestColor() const {
    const CardColor order[4] = {
        CardColor::Red,
        CardColor::Blue,
        CardColor::Green,
        CardColor::Yellow
    };

    int counts[4] = {0, 0, 0, 0};
    Node<Card>* current = getHand().getHead();

    while (current != nullptr) {
        const Card& card = current->data;

        if (!card.isWildCard()) {
            for (int i = 0; i < 4; ++i) {
                if (card.getColor() == order[i]) {
                    ++counts[i];
                }
            }
        }

        current = current->next;
    }

    int best = 0;

    for (int i = 1; i < 4; ++i) {
        if (counts[i] > counts[best]) {
            best = i;
        }
    }

    return order[best];
}

int BotPlayer::findPenaltyCard() const {
    Card card;

    for (int i = 0; i < getHandSize(); ++i) {
        getCard(i, card);

        if (RuleManager::penaltyValue(card) > 0) {
            return i;
        }
    }

    return -1;
}

int BotPlayer::findNormalCard(const RoundState& state) const {
    CustomStack<int> noPenalty;
    Card card;
    int wildIndex = -1;
    int plusFourIndex = -1;

    for (int i = 0; i < getHandSize(); ++i) {
        getCard(i, card);

        if (card.getType() == CardType::Wild) {
            if (wildIndex < 0) {
                wildIndex = i;
            }
        } else if (card.getType() == CardType::WildDrawFour) {
            if (plusFourIndex < 0 &&
                RuleManager::isValidPlay(card, state.topCard,
                                         state.activeColor, getHand(),
                                         noPenalty)) {
                plusFourIndex = i;
            }
        } else if (RuleManager::isValidPlay(card, state.topCard,
                                            state.activeColor, getHand(),
                                            noPenalty)) {
            return i;
        }
    }

    if (wildIndex >= 0) {
        return wildIndex;
    }

    return plusFourIndex;
}

Action BotPlayer::chooseAction(const RoundState& state) {
    Card card;

    if (state.hasDrawn) {
        int last = getHandSize() - 1;

        if (!getCard(last, card)) {
            return Action(ActionType::Pass);
        }

        CardColor color = card.isWildCard() ? chooseBestColor()
                                            : CardColor::None;

        return Action(ActionType::PlayDrawn, last, color);
    }

    if (state.pendingPenalty > 0) {
        int index = findPenaltyCard();

        if (index < 0) {
            return Action(ActionType::Draw);
        }

        getCard(index, card);

        CardColor color = card.isWildCard() ? chooseBestColor()
                                            : CardColor::None;

        return Action(ActionType::Play, index, color);
    }

    int index = findNormalCard(state);

    if (index < 0) {
        return Action(ActionType::Draw);
    }

    getCard(index, card);

    CardColor color = card.isWildCard() ? chooseBestColor()
                                        : CardColor::None;

    return Action(ActionType::Play, index, color);
}