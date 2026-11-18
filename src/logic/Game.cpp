#include "Game.h"

Game::Game(const DynamicArray<Player*>& seats, GameMode gameMode,
           int gameLimit)
    : mode(gameMode),
      limit(gameLimit),
      roundsPlayed(0),
      finished(false),
      gameWinner(nullptr),
      round(nullptr),
      roundScored(false),
      lastRoundPoints(0) 
{
    for (int i = 0; i < seats.size(); ++i) 
    {
        Player* player = nullptr;
        seats.get(i, player);
        players.add(player);
    }
}

Game::~Game()
{
    delete round;
}

bool Game::isValidSetup() const 
{
    return players.size() >= 2 && players.size() <= 10 && limit > 0;
}

bool Game::start() 
{
    if (!isValidSetup()) 
    {
        return false;
    }

    for (int i = 0; i < players.size(); ++i) 
    {
        Player* player = nullptr;
        players.get(i, player);
        player->resetForNewGame();
    }

    roundsPlayed = 0;
    finished = false;
    gameWinner = nullptr;
    roundScored = false;
    lastRoundPoints = 0;

    delete round;
    round = new Round(players);
    round->start();

    return true;
}

bool Game::finishRound() 
{
    if (round == nullptr || !round->isOver() || roundScored) 
    {
        return false;
    }

    lastRoundPoints = ScoreManager::recordRound(round->getWinner(), players);
    ++roundsPlayed;
    roundScored = true;

    Player* winner = ScoreManager::findGameWinner(mode, limit, roundsPlayed,
                                                  players);

    if (winner != nullptr) 
    {
        finished = true;
        gameWinner = winner;
        ScoreManager::recordGameResults(players, winner);
    }

    return true;
}

bool Game::startNextRound() 
{
    if (round == nullptr || !roundScored || finished) 
    {
        return false;
    }

    delete round;
    round = new Round(players);
    round->start();
    roundScored = false;

    return true;
}

bool Game::stepBot(Action& performed, MoveResult& result) 
{
    if (round == nullptr || round->isOver()) 
    {
        return false;
    }

    Player* current = round->getCurrentPlayer();

    if (current == nullptr || current->isHuman()) 
    {
        return false;
    }

    if (round->needsStartColor()) 
    {
        CardColor color = CardColor::Red;
        BotPlayer* bot = dynamic_cast<BotPlayer*>(current);

        if (bot != nullptr) 
        {
            color = bot->chooseBestColor();
        }

        round->setStartColor(color);
        performed = Action(ActionType::None, -1, color);
        result = MoveResult::Ok;

        return true;
    }

    performed = current->chooseAction(round->getState());
    result = round->applyAction(performed);

    if (result != MoveResult::Ok) 
    {
        performed = Action(ActionType::Draw);
        result = round->applyAction(performed);

        if (result != MoveResult::Ok) 
        {
            performed = Action(ActionType::Pass);
            result = round->applyAction(performed);
        }
    }

    return true;
}

Round* Game::getRound() const 
{
    return round;
}

GameMode Game::getMode() const 
{
    return mode;
}

int Game::getLimit() const 
{
    return limit;
}

int Game::getRoundsPlayed() const 
{
    return roundsPlayed;
}

int Game::getLastRoundPoints() const 
{
    return lastRoundPoints;
}

int Game::getPlayerCount() const 
{
    return players.size();
}

Player* Game::getPlayerAt(int index) const 
{
    Player* player = nullptr;
    players.get(index, player);
    return player;
}

bool Game::isFinished() const 
{
    return finished;
}

Player* Game::getGameWinner() const 
{
    return gameWinner;
}