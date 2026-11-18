#ifndef BOT_PLAYER_H
#define BOT_PLAYER_H

#include "Player.h"

class BotPlayer : public Player {
private:
    int findPenaltyCard() const;
    int findNormalCard(const RoundState& state) const;

public:
    BotPlayer(const string& playerName);

    bool isHuman() const override;
    Action chooseAction(const RoundState& state) override;
    CardColor chooseBestColor() const;
};

#endif