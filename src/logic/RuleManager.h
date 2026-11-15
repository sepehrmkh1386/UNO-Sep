#ifndef RULE_MANAGER_H
#define RULE_MANAGER_H

#include "../ds/CustomStack.h"
#include "../ds/SinglyLinkedList.h"
#include "../model/Card.h"

class RuleManager {
private:
    static bool hasColorCard(const SinglyLinkedList<Card>& hand,
                             CardColor activeColor);
    static bool matchesTopCard(const Card& card, const Card& topCard,
                               CardColor activeColor);

public:
    static bool isValidPlay(const Card& card,
                            const Card& topCard,
                            CardColor activeColor,
                            const SinglyLinkedList<Card>& hand,
                            const CustomStack<int>& penaltyStack);

    static int penaltyValue(const Card& card);
};

#endif