#include "GuiController.h"

#include <QTimer>

#include "../model/BotPlayer.h"
#include "../model/HumanPlayer.h"
#include "../view/ConsoleView.h"

namespace {

QString cardsPhrase(int count) {
    if (count == 1) {
        return "1 card";
    }

    return QString::number(count) + " cards";
}

QString colorNameText(CardColor color) {
    switch (color) {
        case CardColor::Red:
            return "Red";
        case CardColor::Yellow:
            return "Yellow";
        case CardColor::Green:
            return "Green";
        case CardColor::Blue:
            return "Blue";
        case CardColor::None:
        default:
            return "None";
    }
}

}  // namespace

GuiController::GuiController(AccountManager& accounts, QObject* parent)
    : QObject(parent),
      accounts_(accounts),
      current_(nullptr),
      game_(nullptr),
      waitingStartColor_(false) {
    // The console AppController::run() loads the account file before the
    // menu; the GUI coordinator does the same here so both entry points
    // share the exact persistence behavior.
    accounts_.load();

    botTimer_ = new QTimer(this);
    botTimer_->setSingleShot(true);
    botTimer_->setInterval(900);

    connect(botTimer_, &QTimer::timeout, this, &GuiController::runBotTurn);
}

GuiController::~GuiController() {
    stopBotTimer();

    if (game_ != nullptr) {
        delete game_;
        game_ = nullptr;
    }

    clearSeats();
}

Account* GuiController::currentAccount() const {
    return current_;
}

int GuiController::accountCount() const {
    return accounts_.size();
}

const Account* GuiController::accountAt(int index) const {
    return accounts_.getAt(index);
}

void GuiController::registerAccount(const string& username,
                                    const string& password,
                                    const string& displayName) {
    RegisterResult result =
        accounts_.registerAccount(username, password, displayName);

    if (result == RegisterResult::Ok) {
        emit errorRaised("");
        emit accountsChanged();
        emit logLine("Account created. You can log in now.");
        return;
    }

    QString message;

    switch (result) {
        case RegisterResult::InvalidUsername:
            message = "Invalid username. Use 3-16 letters, digits or '_' "
                      "starting with a letter.";
            break;
        case RegisterResult::UsernameTaken:
            message = "That username is already taken.";
            break;
        case RegisterResult::WeakPassword:
            message = "Weak password. Use at least 6 characters.";
            break;
        case RegisterResult::InvalidDisplayName:
            message = "Invalid display name (1-24 characters).";
            break;
        case RegisterResult::ForbiddenCharacter:
            message = "Usernames may not contain '|' or spaces.";
            break;
        default:
            message = "The account could not be created.";
            break;
    }

    emit errorRaised(message);
}

void GuiController::login(const string& username, const string& password) {
    Account* account = accounts_.login(username, password);

    if (account == nullptr) {
        emit errorRaised("Wrong username or password.");
        return;
    }

    current_ = account;
    emit errorRaised("");
    emit accountsChanged();
    emit logLine("Welcome, " +
                 QString::fromStdString(account->getDisplayName()) + "!");
}

void GuiController::logout() {
    current_ = nullptr;
    emit accountsChanged();
}

QString GuiController::leaderboardText() const {
    DynamicArray<Account*> ranked;
    accounts_.ranked(ranked);

    // Reuse the existing pure formatting helper from the view layer.
    return QString::fromStdString(ConsoleView::formatLeaderboard(ranked));
}

bool GuiController::startGame(int humans,
                              const DynamicArray<string>& extraUsernames,
                              int bots, GameMode mode, int limit) {
    if (current_ == nullptr) {
        emit errorRaised("Please log in first.");
        return false;
    }

    if (humans < 1 || humans > 4) {
        emit errorRaised("The GUI supports 1 to 4 human players.");
        return false;
    }

    if (bots < 0 || bots > 8) {
        emit errorRaised("The GUI supports 0 to 8 robot players.");
        return false;
    }

    if (humans - 1 != static_cast<int>(extraUsernames.size())) {
        emit errorRaised("Choose an account for every extra human player.");
        return false;
    }

    if (limit <= 0) {
        emit errorRaised("The score or round limit must be positive.");
        return false;
    }

    DynamicArray<Account*> usedAccounts;
    usedAccounts.add(current_);

    clearSeats();
    seats_.add(new HumanPlayer(current_->getDisplayName(), current_));

    for (int i = 0; i < extraUsernames.size(); ++i) {
        string username;
        extraUsernames.get(i, username);

        Account* account = accounts_.find(username);

        if (account == nullptr) {
            clearSeats();
            emit errorRaised("No registered user with that username.");
            return false;
        }

        bool duplicated = false;

        for (int k = 0; k < usedAccounts.size(); ++k) {
            Account* other = nullptr;
            usedAccounts.get(k, other);

            if (other == account) {
                duplicated = true;
                break;
            }
        }

        if (duplicated) {
            clearSeats();
            emit errorRaised("That account is already in the game.");
            return false;
        }

        usedAccounts.add(account);
        seats_.add(new HumanPlayer(account->getDisplayName(), account));
    }

    for (int i = 1; i <= bots; ++i) {
        seats_.add(new BotPlayer("Bot_" + to_string(i)));
    }

    if (seats_.size() < 2 || seats_.size() > 10) {
        clearSeats();
        emit errorRaised("A game needs 2 to 10 players.");
        return false;
    }

    stopBotTimer();

    if (game_ != nullptr) {
        delete game_;
        game_ = nullptr;
    }

    game_ = new Game(seats_, mode, limit);

    if (!game_->start()) {
        delete game_;
        game_ = nullptr;
        clearSeats();
        emit errorRaised("The game could not be started.");
        return false;
    }

    waitingStartColor_ = false;

    emit errorRaised("");
    emit gameStarted();

    afterEngineAction("Round 1 begins. Good luck!");

    return true;
}

void GuiController::leaveGame() {
    stopBotTimer();
    waitingStartColor_ = false;

    if (game_ != nullptr) {
        delete game_;
        game_ = nullptr;
    }

    clearSeats();

    emit gameClosed();
    emit tableChanged();
}

void GuiController::nextRound() {
    if (!gameActive()) {
        return;
    }

    Round* round = game_->getRound();

    if (round == nullptr || !round->isOver()) {
        return;
    }

    if (!game_->startNextRound()) {
        return;
    }

    waitingStartColor_ = false;

    QString line = "Round " + QString::number(game_->getRoundsPlayed() + 1) +
                   " begins.";
    afterEngineAction(line);
}

bool GuiController::gameActive() const {
    return game_ != nullptr && game_->getRound() != nullptr;
}

QString GuiController::localPlayerName() const {
    if (game_ == nullptr || game_->getPlayerCount() == 0) {
        return QString();
    }

    Player* local = game_->getPlayerAt(0);

    if (local == nullptr) {
        return QString();
    }

    return QString::fromStdString(local->getName());
}

void GuiController::playCard(int cardIndex, CardColor chosenColor) {
    if (!gameActive()) {
        return;
    }

    Round* round = game_->getRound();
    Player* mover = round->getCurrentPlayer();

    MoveResult result = round->playCard(cardIndex, chosenColor);

    if (result != MoveResult::Ok) {
        handleRejectedMove(result);
        return;
    }

    QString name = QString::fromStdString(mover->getName());
    QString line;
    Card top;

    if (round->getTopCard(top)) {
        line = name + " played " + cardText(top);
    } else {
        line = name + " played a card.";
    }

    if (round->getPendingPenalty() > 0) {
        line += " — pending penalty: " +
                QString::number(round->getPendingPenalty());
    }

    afterEngineAction(line);
}

void GuiController::playDrawn(CardColor chosenColor) {
    if (!gameActive()) {
        return;
    }

    Round* round = game_->getRound();
    Player* mover = round->getCurrentPlayer();

    MoveResult result = round->playDrawn(chosenColor);

    if (result != MoveResult::Ok) {
        handleRejectedMove(result);
        return;
    }

    Card top;
    round->getTopCard(top);

    afterEngineAction(QString::fromStdString(mover->getName()) +
                      " played the drawn card: " + cardText(top));
}

void GuiController::drawCard() {
    if (!gameActive()) {
        return;
    }

    Round* round = game_->getRound();
    Player* mover = round->getCurrentPlayer();

    MoveResult result = round->draw();

    if (result != MoveResult::Ok) {
        handleRejectedMove(result);
        return;
    }

    QString name = QString::fromStdString(mover->getName());
    QString line;

    Card drawn;

    if (round->hasDrawnCard() && round->getLastDrawnCard(drawn)) {
        line = name + " drew " + cardText(drawn) +
               ". It can be played — play it or pass.";
    } else if (round->getLastDrawCount() == 1 &&
               round->getLastDrawnCard(drawn)) {
        line = name + " drew " + cardText(drawn) +
               ". It cannot be played — the turn ends.";
    } else if (round->getLastDrawCount() == 0) {
        line = name + " wanted to draw, but there are no cards left. "
                    "The turn ends.";
    } else {
        line = name + " drew " + cardsPhrase(round->getLastDrawCount()) +
               " and the turn ends.";
    }

    afterEngineAction(line);
}

void GuiController::passTurn() {
    if (!gameActive()) {
        return;
    }

    Round* round = game_->getRound();
    Player* mover = round->getCurrentPlayer();

    MoveResult result = round->pass();

    if (result != MoveResult::Ok) {
        handleRejectedMove(result);
        return;
    }

    afterEngineAction(QString::fromStdString(mover->getName()) + " passed.");
}

void GuiController::useChance() {
    if (!gameActive()) {
        return;
    }

    Round* round = game_->getRound();
    Player* mover = round->getCurrentPlayer();

    MoveResult result = round->chance();

    if (result == MoveResult::NotAllowed) {
        emit statusLine("The chance machine can only be used while a draw "
                        "penalty is pending.");
        return;
    }

    if (result != MoveResult::Ok) {
        handleRejectedMove(result);
        return;
    }

    afterEngineAction(QString::fromStdString(mover->getName()) +
                      " used the chance machine and got " +
                      cardsPhrase(round->getLastDrawCount()) +
                      ". The turn ends.");
}

void GuiController::chooseStartColor(CardColor color) {
    waitingStartColor_ = false;

    if (!gameActive()) {
        return;
    }

    Round* round = game_->getRound();

    if (!round->setStartColor(color)) {
        emit statusLine("A starting color cannot be chosen right now.");
        return;
    }

    Player* mover = round->getCurrentPlayer();
    QString name = (mover != nullptr)
                       ? QString::fromStdString(mover->getName())
                       : QString("The player");

    afterEngineAction(name + " chose the starting color: " +
                      colorNameText(color) + ".");
}

QString GuiController::roundSummaryText() const {
    if (!gameActive()) {
        return QString();
    }

    Round* round = game_->getRound();
    Player* winner = round->getWinner();

    QString text = "<b>Round " + QString::number(game_->getRoundsPlayed()) +
                   " finished.</b><br><br>";

    if (winner != nullptr) {
        text += "<b>" + QString::fromStdString(winner->getName()) +
                "</b> won the round and received <b>" +
                QString::number(game_->getLastRoundPoints()) + "</b> points."
                "<br><br>";
    }

    text += "<table cellpadding='4' cellspacing='0'>"
            "<tr><td><b>Player</b></td><td><b>Hand score</b></td>"
            "<td><b>Game score</b></td></tr>";

    for (int i = 0; i < game_->getPlayerCount(); ++i) {
        Player* player = game_->getPlayerAt(i);

        text += "<tr><td>" + QString::fromStdString(player->getName()) +
                (player == winner ? QString(" &starf;") : QString("")) +
                "</td><td>" + QString::number(player->handScore()) +
                "</td><td>" + QString::number(player->getCurrentGameScore()) +
                "</td></tr>";
    }

    text += "</table><br>" + winConditionText();

    return text;
}

QString GuiController::gameOverText() const {
    if (game_ == nullptr) {
        return QString();
    }

    Player* winner = game_->getGameWinner();

    QString text = "<b>The game is finished.</b><br><br>";

    if (winner != nullptr) {
        text += "<b>" + QString::fromStdString(winner->getName()) +
                "</b> wins the game!<br><br>";
    }

    text += "<table cellpadding='4' cellspacing='0'>"
            "<tr><td><b>Player</b></td><td><b>Game score</b></td></tr>";

    for (int i = 0; i < game_->getPlayerCount(); ++i) {
        Player* player = game_->getPlayerAt(i);

        text += "<tr><td>" + QString::fromStdString(player->getName()) +
                "</td><td>" + QString::number(player->getCurrentGameScore()) +
                "</td></tr>";
    }

    text += "</table><br>Results have been saved to the accounts file.";

    return text;
}

QString GuiController::winConditionText() const {
    if (game_ == nullptr) {
        return QString();
    }

    if (game_->getMode() == GameMode::Score) {
        return "Score mode — first to " + QString::number(game_->getLimit()) +
               " points wins.";
    }

    return "Rounds mode — " + QString::number(game_->getLimit()) +
           " rounds will be played.";
}

void GuiController::afterEngineAction(const QString& successLogLine) {
    if (!successLogLine.isEmpty()) {
        emit logLine(successLogLine);
        emit statusLine(successLogLine);
    }

    emit tableChanged();

    if (!gameActive()) {
        return;
    }

    Round* round = game_->getRound();

    if (round->isOver()) {
        stopBotTimer();
        waitingStartColor_ = false;

        // Existing engine scoring: records round points, checks the win
        // condition and records game results for accounts.
        game_->finishRound();

        emit tableChanged();

        if (game_->isFinished()) {
            accounts_.save();
            emit accountsChanged();
            emit gameOverReady();
        } else {
            emit roundOverReady();
        }

        return;
    }

    Player* current = round->getCurrentPlayer();

    if (current != nullptr && current->isHuman()) {
        if (round->needsStartColor() && !waitingStartColor_) {
            waitingStartColor_ = true;
            emit needStartColorChoice();
        } else {
            emit statusLine(QString::fromStdString(current->getName()) +
                            ", it is your turn.");
        }
    }

    scheduleBotTurn();
}

void GuiController::scheduleBotTurn() {
    if (!gameActive()) {
        stopBotTimer();
        return;
    }

    Round* round = game_->getRound();
    Player* current = round->getCurrentPlayer();

    if (current != nullptr && !current->isHuman() && !round->isOver()) {
        emit statusLine(QString::fromStdString(current->getName()) +
                        " is thinking...");
        botTimer_->start();
        return;
    }

    stopBotTimer();
}

void GuiController::stopBotTimer() {
    botTimer_->stop();
}

void GuiController::runBotTurn() {
    if (!gameActive()) {
        stopBotTimer();
        return;
    }

    Round* round = game_->getRound();
    Player* mover = round->getCurrentPlayer();

    if (mover == nullptr || mover->isHuman() || round->isOver()) {
        stopBotTimer();
        return;
    }

    Action action;
    MoveResult result = MoveResult::Ok;

    // Existing engine bot logic (BotPlayer::chooseAction + fallbacks).
    if (!game_->stepBot(action, result)) {
        stopBotTimer();
        return;
    }

    afterEngineAction(botMoveText(*mover, action, result));
}

void GuiController::clearSeats() {
    while (!seats_.isEmpty()) {
        Player* player = nullptr;
        seats_.get(seats_.size() - 1, player);
        delete player;
        seats_.removeLast();
    }
}

void GuiController::handleRejectedMove(MoveResult result) {
    emit statusLine(describeMoveResult(result));

    // A rejected move can still reveal that the round just ended (for
    // example when the previous play emptied a hand); process the round
    // finish exactly like a successful action would.
    if (result == MoveResult::RoundOver) {
        afterEngineAction(QString());
    }
}

QString GuiController::describeMoveResult(MoveResult result) const {
    switch (result) {
        case MoveResult::Ok:
            return QString();
        case MoveResult::InvalidIndex:
            return "That card is not in your hand.";
        case MoveResult::InvalidMove:
            if (gameActive() && game_->getRound()->getPendingPenalty() > 0) {
                return "Only +2 / +4 cards can be played while a draw penalty "
                       "is pending (stacking).";
            }

            return "This card cannot be played on the current top card.";
        case MoveResult::ColorRequired:
            return "Choose a color for this Wild card.";
        case MoveResult::AwaitingStartColor:
            return "The first card is a Wild card — choose the starting "
                   "color first.";
        case MoveResult::RoundOver:
            return "The round is already over.";
        case MoveResult::NotAllowed:
            if (gameActive() && game_->getRound()->hasDrawnCard()) {
                return "You already drew a card: play it or pass.";
            }

            return "This action is not allowed right now.";
    }

    return "This action is not allowed right now.";
}

QString GuiController::botMoveText(const Player& mover, const Action& action,
                                   MoveResult result) const {
    QString name = QString::fromStdString(mover.getName());

    if (result != MoveResult::Ok) {
        return name + " could not act (" + describeMoveResult(result) + ")";
    }

    Round* round = game_->getRound();

    switch (action.type) {
        case ActionType::None: {
            return name + " chose the starting color: " +
                   colorNameText(action.color) + ".";
        }
        case ActionType::Play: {
            Card top;

            if (round != nullptr && round->getTopCard(top)) {
                QString line = name + " played " + cardText(top);

                if (round->getPendingPenalty() > 0) {
                    line += " — pending penalty: " +
                            QString::number(round->getPendingPenalty());
                }

                return line;
            }

            return name + " played a card.";
        }
        case ActionType::PlayDrawn: {
            Card top;

            if (round != nullptr && round->getTopCard(top)) {
                return name + " played the drawn card: " + cardText(top);
            }

            return name + " played the drawn card.";
        }
        case ActionType::Draw: {
            int count = (round != nullptr) ? round->getLastDrawCount() : 0;

            if (count == 0) {
                return name + " wanted to draw, but the deck is empty.";
            }

            return name + " drew " + cardsPhrase(count) + ".";
        }
        case ActionType::Pass:
            return name + " passed.";
        case ActionType::Chance:
            return name + " used the chance machine.";
    }

    return name + " moved.";
}

// Local helper: converts an engine Card into presentation text. Implemented
// as a small member-like helper through the UnoText namespace in the header.
QString GuiController::cardText(const Card& card) const {
    return QString::fromStdString(card.toString());
}
