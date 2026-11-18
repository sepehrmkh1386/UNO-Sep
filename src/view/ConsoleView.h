#ifndef CONSOLE_VIEW_H
#define CONSOLE_VIEW_H

#include <string>

#include "../ds/DynamicArray.h"
#include "../logic/AccountManager.h"
#include "../logic/Game.h"
#include "../logic/Round.h"
#include "../model/Account.h"
#include "../model/Player.h"
#include "../model/RoundState.h"

using namespace std;

class ConsoleView {
public:
    static string formatHand(const Player& player);
    static string formatTable(const Game& game, const Round& round);
    static string formatScores(const Game& game);
    static string formatRoundSummary(const Game& game);
    static string formatLeaderboard(const DynamicArray<Account*>& ranked);
    static string formatBotMove(const Player& bot, const Action& action,
                                const Round& round);

    static string describeResult(MoveResult result);
    static string describeRegisterResult(RegisterResult result);

    static string authHelp();
    static string mainHelp();
    static string gameHelp();

    static void show(const string& text);
    static bool readLine(const string& prompt, string& line);
};

#endif