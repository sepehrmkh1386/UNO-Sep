#ifndef SCORE_MANAGER_H
#define SCORE_MANAGER_H

#include "../ds/DynamicArray.h"
#include "../model/Player.h"

enum class GameMode 
{
    Score,
    Rounds
};

class ScoreManager 
{
public:
    static int calcRoundPoints(const Player* winner,
                               const DynamicArray<Player*>& players);

    static int recordRound(Player* winner,
                           const DynamicArray<Player*>& players);

    static Player* findGameWinner(GameMode mode, int limit, int roundsPlayed,
                                  const DynamicArray<Player*>& players);

    static void recordGameResults(const DynamicArray<Player*>& players,
                                  Player* gameWinner);
};

#endif