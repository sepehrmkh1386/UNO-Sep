#include "ScoreManager.h"

int ScoreManager::calcRoundPoints(const Player* winner, const DynamicArray<Player*>& players)
{
    int total = 0;
    for (int i=0;i<players.size();i++)
    {
        Player* player = nullptr;
        players.get(i, player);

        if(player != winner)
        {
             total += player->handScore();
        }
    }
    return total;
}

int ScoreManager::recordRound(Player* winner, const DynamicArray<Player*>& players) {
    int points = calcRoundPoints(winner, players);

    for (int i = 0; i < players.size(); ++i) 
    {
        Player* player = nullptr;
        players.get(i, player);

        if (player == winner) 
        {
            player->recordRoundScore(points);
        } else {
            player->recordRoundScore(0);
        }
    }

    return points;
}

Player* ScoreManager::findGameWinner(GameMode mode, int limit, int roundsPlayed, const DynamicArray<Player*>&players)
{
    if(players.isEmpty())
    {
        return nullptr;
    }

    if (mode == GameMode::Rounds && roundsPlayed < limit)
    {
        return nullptr;
    }

    Player* best = nullptr;
    int bestScore = -1;
    int bestCount = 0;

    for(int i=0; i<players.size(); ++i)
    {
        Player* player =nullptr;
        players.get(i, player);
        

        int score = player->getCurrentGameScore();

        if (score > bestScore)
        {
            best = player;
            bestScore = score;
            bestCount = 1;
        }
        else if (score == bestScore)
        {
            ++bestCount;
        }
    }

    if (mode == GameMode::Score)
    {
        if (bestScore >= limit)
        {
            return best;
        }

        return nullptr;
    }

    if (bestCount > 1)
    {
        return nullptr;
    }

    return best;
}

void ScoreManager::recordGameResults(const DynamicArray<Player*>& players, Player* gameWinner)
{
    for(int i=0 ; i<players.size() ; ++i)
    {
        Player* player = nullptr;
        players.get(i, player);

        Account* account = player->getAccount();

        if(account != nullptr)
        {
            account->recordGame(player->getCurrentGameScore(), player == gameWinner);
        }
    }
}