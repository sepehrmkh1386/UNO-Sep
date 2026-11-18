#ifndef COMMAND_H
#define COMMAND_H

#include <string>

#include "../logic/ScoreManager.h"
#include "../model/Card.h"

using namespace std;

enum class CommandType {
    Empty,
    Unknown,
    Register,
    Login,
    Leaderboard,
    StartGame,
    Logout,
    Play,
    Draw,
    PlayDrawn,
    Pass,
    Chance,
    Scores,
    QuitGame,
    CheatGive,
    CheatTop,
    CheatEmptyDeck,
    CheatWinRound
};

struct Command {
    CommandType type;
    bool valid;
    string error;

    string username;
    string password;
    string displayName;

    int humans;
    int bots;
    int limit;
    GameMode mode;

    int cardIndex;
    CardColor color;
    bool hasColor;

    Card cheatCard;

    Command()
        : type(CommandType::Empty), valid(false), error(""),
          username(""), password(""), displayName(""),
          humans(0), bots(0), limit(0), mode(GameMode::Score),
          cardIndex(-1), color(CardColor::None), hasColor(false),
          cheatCard() {
    }
};

#endif