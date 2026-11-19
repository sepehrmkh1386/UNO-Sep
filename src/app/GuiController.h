#ifndef GUI_CONTROLLER_H
#define GUI_CONTROLLER_H

// Application-layer coordinator for the Qt GUI.
//
// This class is the integration boundary between the Qt Widgets UI and the
// EXISTING game engine (Game / Round / AccountManager). It performs no rule
// logic of its own: every account operation, every move validation, every
// bot decision, every penalty and every Device-of-Chance roll is performed
// by the existing engine classes. The controller only:
//   1. forwards UI requests to the engine,
//   2. reads back the engine state,
//   3. paces bot turns with a QTimer so the UI thread never blocks,
//   4. translates engine results into presentation messages.

#include <QObject>
#include <QString>
#include <QTimer>

#include "../ds/DynamicArray.h"
#include "../logic/AccountManager.h"
#include "../logic/Game.h"
#include "../logic/ScoreManager.h"
#include "../model/Account.h"
#include "../model/Card.h"
#include "../model/RoundState.h"

using namespace std;

class Player;

class GuiController : public QObject {
    Q_OBJECT

public:
    explicit GuiController(AccountManager& accounts,
                           QObject* parent = nullptr);
    ~GuiController() override;

    GuiController(const GuiController&) = delete;
    GuiController& operator=(const GuiController&) = delete;

    // --- accounts (forwarded to the existing AccountManager) ---
    Account* currentAccount() const;
    int accountCount() const;
    const Account* accountAt(int index) const;
    void registerAccount(const string& username, const string& password,
                         const string& displayName);
    void login(const string& username, const string& password);
    void logout();
    QString leaderboardText() const;

    // --- game lifecycle ---
    bool startGame(int humans, const DynamicArray<string>& extraUsernames,
                   int bots, GameMode mode, int limit);
    void leaveGame();
    void nextRound();
    bool gameActive() const;

    // Engine access for the views (read-only usage) and for tests.
    const Game* game() const { return game_; }
    Game* game() { return game_; }

    QString localPlayerName() const;

    // --- human actions (1:1 delegation to the existing Round methods) ---
    void playCard(int cardIndex, CardColor chosenColor);
    void playDrawn(CardColor chosenColor);
    void drawCard();
    void passTurn();
    void useChance();
    void chooseStartColor(CardColor color);

    // --- presentation helpers (pure reads of engine state) ---
    QString roundSummaryText() const;
    QString gameOverText() const;
    QString winConditionText() const;

signals:
    void errorRaised(const QString& message);
    void accountsChanged();
    void gameStarted();
    void gameClosed();
    void tableChanged();
    void logLine(const QString& line);
    void statusLine(const QString& line);
    void roundOverReady();
    void gameOverReady();
    void needStartColorChoice();

private slots:
    void runBotTurn();

private:
    void afterEngineAction(const QString& successLogLine);
    void scheduleBotTurn();
    void stopBotTimer();
    void clearSeats();

    QString describeMoveResult(MoveResult result) const;
    void handleRejectedMove(MoveResult result);
    QString botMoveText(const Player& mover, const Action& action,
                        MoveResult result) const;
    QString cardText(const Card& card) const;

    AccountManager& accounts_;
    Account* current_;
    Game* game_;
    DynamicArray<Player*> seats_;
    QTimer* botTimer_;
    bool waitingStartColor_;
};

#endif
