#ifndef APP_CONTROLLER_H
#define APP_CONTROLLER_H

#include <string>

#include "../logic/AccountManager.h"
#include "../logic/Game.h"
#include "../model/Account.h"
#include "../view/Command.h"

using namespace std;

class AppController {
private:
    AccountManager& accounts;
    Account* currentAccount;
    bool inputClosed;

    bool ask(const string& prompt, string& line);

    bool authStep();
    void mainStep();
    void showLeaderboard();

    void startGame(const Command& command);
    void playGame(Game& game, int humanCount);
    bool botTurn(Round& round, Game& game);
    bool humanTurn(Game& game, Round& round, int humanCount,
                   Player*& lastHuman, bool& needTable);
    bool handleGameCommand(const Command& command, Game& game, Round& round,
                           bool& needTable);

public:
    AppController(AccountManager& manager);

    AppController(const AppController&) = delete;
    AppController& operator=(const AppController&) = delete;

    void run();
};

#endif