#ifndef GAME_H
#define GAME_H

#include "../ds/DynamicArray.h"
#include "../model/BotPlayer.h"
#include "../model/Player.h"
#include "../model/RoundState.h"
#include "Round.h"
#include "ScoreManager.h"

class Game 
{
private:
    DynamicArray<Player*> players;
    GameMode mode;
    int limit;
    int roundsPlayed;
    bool finished;
    Player* gameWinner;
    Round* round;
    bool roundScored;
    int lastRoundPoints;

public:
    Game(const DynamicArray<Player*>& seats, GameMode gameMode, int gameLimit);
    ~Game();

    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    bool isValidSetup() const;
    bool start();
    bool finishRound();
    bool startNextRound();
    bool stepBot(Action& performed, MoveResult& result);

    Round* getRound() const;
    GameMode getMode() const;
    int getLimit() const;
    int getRoundsPlayed() const;
    int getLastRoundPoints() const;
    int getPlayerCount() const;
    Player* getPlayerAt(int index) const;
    bool isFinished() const;
    Player* getGameWinner() const;
};

#endif