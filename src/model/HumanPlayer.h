#ifndef HUMAN_PLAYER_H
#define HUMAN_PLAYER_H

#include "Player.h"

class HumanPlayer : public Player {
private:
    Action nextAction;

public:
    HumanPlayer(const string& playerName, Account* playerAccount = nullptr)
        : Player(playerName, playerAccount), nextAction() {
    }

    bool isHuman() const override {
        return true;
    }

    void setNextAction(const Action& action) {
        nextAction = action;
    }

    Action chooseAction(const RoundState&) override {
        Action result = nextAction;
        nextAction = Action();
        return result;
    }
};

#endif