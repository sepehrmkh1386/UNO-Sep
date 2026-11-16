#include "BotPlayer.h"

BotPlayer::BotPlayer(const string& playerName)
    : Player(playerName, nullptr) {
}

bool BotPlayer::isHuman() const {
    return false;
}

Action BotPlayer::chooseAction(const RoundState&) {
    return Action(ActionType::Draw);
}