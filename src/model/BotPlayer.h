#ifndef BOT_PLAYER_H
#define BOT_PLAYER_H

#include "Player.h"

class BotPlayer : public Player {
public:
    BotPlayer(const string& playerName);

    bool isHuman() const override;
    Action chooseAction(const RoundState& state) override;
};

#endif