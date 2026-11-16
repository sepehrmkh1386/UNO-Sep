#ifndef ROUND_STATE_H
#define ROUND_STATE_H

#include "Card.h"

enum class ActionType {
    None,
    Play,
    Draw,
    PlayDrawn,
    Pass,
    Chance
};

struct Action {
    ActionType type;
    int cardIndex;
    CardColor color;

    Action()
        : type(ActionType::None), cardIndex(-1), color(CardColor::None) {
    }

    Action(ActionType actionType, int index = -1,
           CardColor chosenColor = CardColor::None)
        : type(actionType), cardIndex(index), color(chosenColor) {
    }
};

struct RoundState {
    Card topCard;
    CardColor activeColor;
    int pendingPenalty;
    bool hasDrawn;

    RoundState()
        : topCard(), activeColor(CardColor::None),
          pendingPenalty(0), hasDrawn(false) {
    }
};

#endif