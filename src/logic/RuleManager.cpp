#include "RuleManager.h"

static bool isActionType(CardType type) {
    return type == CardType::Skip ||
           type == CardType::Reverse ||
           type == CardType::DrawTwo;
}

int RuleManager::penaltyValue(const Card& card) {
    if (card.getType() == CardType::DrawTwo) {
        return 2;
    }

    if (card.getType() == CardType::WildDrawFour) {
        return 4;
    }

    return 0;
}

bool RuleManager::hasColorCard(const SinglyLinkedList<Card>& hand,
                               CardColor activeColor) {
    Node<Card>* current = hand.getHead();

    while (current != nullptr) {
        const Card& card = current->data;

        if (!card.isWildCard() && card.getColor() == activeColor) {
            return true;
        }

        current = current->next;
    }

    return false;
}

bool RuleManager::matchesTopCard(const Card& card, const Card& topCard,
                                 CardColor activeColor) {
    if (card.getColor() == activeColor) {
        return true;
    }

    if (card.isNumberCard() && topCard.isNumberCard() &&
        card.getValue() == topCard.getValue()) {
        return true;
    }

    if (isActionType(card.getType()) &&
        card.getType() == topCard.getType()) {
        return true;
    }

    return false;
}

bool RuleManager::isValidPlay(const Card& card,
                              const Card& topCard,
                              CardColor activeColor,
                              const SinglyLinkedList<Card>& hand,
                              const CustomStack<int>& penaltyStack) {
    if (!penaltyStack.isEmpty()) {
        return penaltyValue(card) > 0;
    }

    if (card.getType() == CardType::Wild) {
        return true;
    }

    if (card.getType() == CardType::WildDrawFour) {
        return !hasColorCard(hand, activeColor);
    }

    return matchesTopCard(card, topCard, activeColor);
}