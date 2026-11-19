#include "AppController.h"

#include "../logic/TextUtils.h"
#include "../model/BotPlayer.h"
#include "../model/HumanPlayer.h"
#include "../view/CommandParser.h"
#include "../view/ConsoleView.h"

static string cardsText(int count) {
    if (count == 1) {
        return "1 card";
    }

    return to_string(count) + " cards";
}

static bool parseColorWord(const string& line, CardColor& color) {
    DynamicArray<string> words;
    splitWords(line, words);

    if (words.size() != 1) {
        return false;
    }

    string upper = toUpperText(wordAt(words, 0));

    if (upper == "RED") {
        color = CardColor::Red;
    } else if (upper == "YELLOW") {
        color = CardColor::Yellow;
    } else if (upper == "GREEN") {
        color = CardColor::Green;
    } else if (upper == "BLUE") {
        color = CardColor::Blue;
    } else {
        return false;
    }

    return true;
}

static bool alreadyUsed(const DynamicArray<Account*>& used, Account* account) {
    for (int i = 0; i < used.size(); ++i) {
        Account* other = nullptr;
        used.get(i, other);

        if (other == account) {
            return true;
        }
    }

    return false;
}

static void freeSeats(DynamicArray<Player*>& seats) {
    for (int i = 0; i < seats.size(); ++i) {
        Player* player = nullptr;
        seats.get(i, player);
        delete player;
    }
}

static void reportMove(MoveResult result, const Round& round,
                       const Player& mover, const Action& action,
                       bool& needTable) {
    if (result == MoveResult::Ok) {
        ConsoleView::show(ConsoleView::formatBotMove(mover, action, round) +
                          "\n");
        needTable = true;
        return;
    }

    if (result == MoveResult::NotAllowed && round.hasDrawnCard()) {
        ConsoleView::show("You drew a playable card: use play_drawn "
                          "[-c <color>] or pass.\n");
        return;
    }

    ConsoleView::show(ConsoleView::describeResult(result) + "\n");
}

AppController::AppController(AccountManager& manager)
    : accounts(manager), currentAccount(nullptr), inputClosed(false) {
}

bool AppController::ask(const string& prompt, string& line) {
    if (inputClosed) {
        return false;
    }

    if (!ConsoleView::readLine(prompt, line)) {
        inputClosed = true;
        ConsoleView::show("\n");
        return false;
    }

    return true;
}

void AppController::run() {
    accounts.load();

    ConsoleView::show("Welcome to UNO!\n");

    if (accounts.getSkippedLines() > 0) {
        ConsoleView::show("Warning: " + to_string(accounts.getSkippedLines()) +
                          " damaged line(s) in the accounts file were "
                          "skipped.\n");
    }

    ConsoleView::show(ConsoleView::authHelp());

    bool running = true;

    while (running && !inputClosed) {
        if (currentAccount == nullptr) {
            running = authStep();
        } else {
            mainStep();
        }
    }

    ConsoleView::show("Goodbye!\n");
}

void AppController::showLeaderboard() {
    DynamicArray<Account*> ranked;
    accounts.ranked(ranked);
    ConsoleView::show(ConsoleView::formatLeaderboard(ranked));
}

bool AppController::authStep() {
    string line;

    if (!ask("uno> ", line)) {
        return false;
    }

    Command command = CommandParser::parse(line);

    if (command.type == CommandType::Empty) {
        return true;
    }

    if (!command.valid) {
        ConsoleView::show(command.error + "\n");

        if (command.type == CommandType::Unknown) {
            ConsoleView::show(ConsoleView::authHelp());
        }

        return true;
    }

    switch (command.type) {
        case CommandType::Register: {
            RegisterResult result = accounts.registerAccount(
                command.username, command.password, command.displayName);

            if (result == RegisterResult::Ok) {
                ConsoleView::show("Account created. You can log in now.\n");
            } else {
                ConsoleView::show(
                    ConsoleView::describeRegisterResult(result) + "\n");
            }
            break;
        }
        case CommandType::Login: {
            Account* account =
                accounts.login(command.username, command.password);

            if (account == nullptr) {
                ConsoleView::show("Wrong username or password.\n");
            } else {
                currentAccount = account;
                ConsoleView::show("Welcome, " + account->getDisplayName() +
                                  "!\n");
                ConsoleView::show(ConsoleView::mainHelp());
            }
            break;
        }
        case CommandType::Leaderboard:
            showLeaderboard();
            break;
        case CommandType::Logout:
            return false;
        default:
            ConsoleView::show("Please login first.\n");
            ConsoleView::show(ConsoleView::authHelp());
            break;
    }

    return true;
}

void AppController::mainStep() {
    string line;

    if (!ask("uno [" + currentAccount->getUsername() + "]> ", line)) {
        return;
    }

    Command command = CommandParser::parse(line);

    if (command.type == CommandType::Empty) {
        return;
    }

    if (!command.valid) {
        ConsoleView::show(command.error + "\n");

        if (command.type == CommandType::Unknown) {
            ConsoleView::show(ConsoleView::mainHelp());
        }

        return;
    }

    switch (command.type) {
        case CommandType::StartGame:
            startGame(command);
            break;
        case CommandType::Leaderboard:
            showLeaderboard();
            break;
        case CommandType::Logout:
            currentAccount = nullptr;
            ConsoleView::show("Logged out.\n");
            ConsoleView::show(ConsoleView::authHelp());
            break;
        case CommandType::Register:
        case CommandType::Login:
            ConsoleView::show("You are already logged in. Use logout first.\n");
            break;
        default:
            ConsoleView::show("That command is only available during a "
                              "game.\n");
            ConsoleView::show(ConsoleView::mainHelp());
            break;
    }
}

void AppController::startGame(const Command& command) {
    DynamicArray<Player*> seats;
    DynamicArray<Account*> used;
    bool cancelled = false;

    used.add(currentAccount);
    seats.add(new HumanPlayer(currentAccount->getDisplayName(),
                              currentAccount));

    for (int i = 2; i <= command.humans && !cancelled; ++i) {
        bool accepted = false;

        while (!accepted && !cancelled) {
            string line;

            if (!ask("Username of human player " + to_string(i) +
                         " (empty line cancels): ",
                     line)) {
                cancelled = true;
                break;
            }

            DynamicArray<string> words;
            splitWords(line, words);

            if (words.size() == 0) {
                cancelled = true;
                break;
            }

            Account* account = accounts.find(wordAt(words, 0));

            if (account == nullptr) {
                ConsoleView::show("No registered user with that username.\n");
                continue;
            }

            if (alreadyUsed(used, account)) {
                ConsoleView::show("This user is already in the game.\n");
                continue;
            }

            used.add(account);
            seats.add(new HumanPlayer(account->getDisplayName(), account));
            accepted = true;
        }
    }

    if (cancelled) {
        ConsoleView::show("The game was cancelled.\n");
        freeSeats(seats);
        return;
    }

    for (int i = 1; i <= command.bots; ++i) {
        seats.add(new BotPlayer("Bot_" + to_string(i)));
    }

    {
        Game game(seats, command.mode, command.limit);

        if (game.start()) {
            playGame(game, command.humans);
        } else {
            ConsoleView::show("The game could not be started.\n");
        }
    }

    freeSeats(seats);
    ConsoleView::show(ConsoleView::mainHelp());
}

void AppController::playGame(Game& game, int humanCount) {
    Player* lastHuman = nullptr;
    bool needTable = true;
    bool quit = false;

    ConsoleView::show("\nThe game starts! Type quit_game to leave it.\n");
    ConsoleView::show(ConsoleView::gameHelp());

    while (!quit && !inputClosed && !game.isFinished()) {
        Round* round = game.getRound();
        needTable = true;

        while (!quit && !inputClosed && !round->isOver()) {
            Player* current = round->getCurrentPlayer();

            if (current->isHuman()) {
                quit = !humanTurn(game, *round, humanCount, lastHuman,
                                  needTable);
            } else {
                quit = !botTurn(*round, game);
            }
        }

        if (quit || !round->isOver()) {
            break;
        }

        game.finishRound();
        ConsoleView::show("\n" + ConsoleView::formatRoundSummary(game) + "\n");

        if (game.isFinished()) {
            accounts.save();
            break;
        }

        ConsoleView::show("Starting the next round...\n");
        game.startNextRound();
    }
}

bool AppController::botTurn(Round& round, Game& game) {
    Player* mover = round.getCurrentPlayer();
    Action action;
    MoveResult result = MoveResult::Ok;

    if (!game.stepBot(action, result)) {
        return false;
    }

    ConsoleView::show(ConsoleView::formatBotMove(*mover, action, round) +
                      "\n");

    return true;
}

bool AppController::humanTurn(Game& game, Round& round, int humanCount,
                              Player*& lastHuman, bool& needTable) {
    Player* current = round.getCurrentPlayer();
    string line;

    if (humanCount > 1 && current != lastHuman) {
        ConsoleView::show(string(40, '\n'));

        if (!ask("Next player: " + current->getName() +
                     ". Press Enter when ready... ",
                 line)) {
            return false;
        }

        needTable = true;
    }

    lastHuman = current;

    if (needTable) {
        ConsoleView::show(ConsoleView::formatTable(game, round));
        needTable = false;
    }

    bool needColor = round.needsStartColor();
    string prompt = "Enter Command > ";

    if (needColor) {
        prompt = "The first card is a Wild card. Choose the starting color "
                 "(RED|YELLOW|GREEN|BLUE) > ";
    }

    if (!ask(prompt, line)) {
        return false;
    }

    if (needColor) {
        CardColor color = CardColor::None;

        if (parseColorWord(line, color) && round.setStartColor(color)) {
            ConsoleView::show("The starting color is " + colorToString(color) +
                              ".\n");
            needTable = true;
            return true;
        }
    }

    Command command = CommandParser::parse(line);

    if (command.type == CommandType::Empty) {
        return true;
    }

    if (!command.valid) {
        ConsoleView::show(command.error + "\n");
        return true;
    }

    return handleGameCommand(command, game, round, needTable);
}

bool AppController::handleGameCommand(const Command& command, Game& game,
                                      Round& round, bool& needTable) {
    Player* mover = round.getCurrentPlayer();
    CardColor color = command.hasColor ? command.color : CardColor::None;

    switch (command.type) {
        case CommandType::QuitGame:
            ConsoleView::show("You left the game. No results were saved.\n");
            return false;
        case CommandType::Scores:
            ConsoleView::show(ConsoleView::formatScores(game));
            break;
        case CommandType::Play: {
            MoveResult result = round.playCard(command.cardIndex, color);
            reportMove(result, round, *mover, Action(ActionType::Play),
                       needTable);
            break;
        }
        case CommandType::Draw: {
            MoveResult result = round.draw();

            if (result != MoveResult::Ok) {
                ConsoleView::show(ConsoleView::describeResult(result) + "\n");
                break;
            }

            Card drawn;
            needTable = true;

            if (round.hasDrawnCard() && round.getLastDrawnCard(drawn)) {
                ConsoleView::show("You drew " + drawn.toString() +
                                  ". It can be played: use play_drawn "
                                  "[-c <color>] or pass.\n");
            } else if (round.getLastDrawCount() == 1 &&
                       round.getLastDrawnCard(drawn)) {
                ConsoleView::show("You drew " + drawn.toString() +
                                  ". It cannot be played, your turn ends.\n");
            } else if (round.getLastDrawCount() == 0) {
                ConsoleView::show("There are no cards left to draw. Your "
                                  "turn ends.\n");
            } else {
                ConsoleView::show("You drew " +
                                  cardsText(round.getLastDrawCount()) +
                                  ". Your turn ends.\n");
            }
            break;
        }
        case CommandType::PlayDrawn: {
            MoveResult result = round.playDrawn(color);
            reportMove(result, round, *mover, Action(ActionType::PlayDrawn),
                       needTable);
            break;
        }
        case CommandType::Pass: {
            MoveResult result = round.pass();
            reportMove(result, round, *mover, Action(ActionType::Pass),
                       needTable);
            break;
        }
        case CommandType::Chance: {
            MoveResult result = round.chance();

            if (result == MoveResult::Ok) {
                ConsoleView::show("The chance machine gives you " +
                                  cardsText(round.getLastDrawCount()) + ".\n");
                needTable = true;
            } else if (result == MoveResult::NotAllowed) {
                ConsoleView::show("The chance machine can only be used while "
                                  "a draw penalty is pending.\n");
            } else {
                ConsoleView::show(ConsoleView::describeResult(result) + "\n");
            }
            break;
        }
        case CommandType::CheatGive:
            if (round.cheatGive(command.cheatCard)) {
                ConsoleView::show("[cheat] Added " +
                                  command.cheatCard.toString() +
                                  " to your hand.\n");
                needTable = true;
            } else {
                ConsoleView::show("[cheat] A card cannot be added now.\n");
            }
            break;
        case CommandType::CheatTop:
            round.cheatSetTop(command.cheatCard);
            ConsoleView::show("[cheat] The top card is now " +
                              command.cheatCard.toString() + ".\n");
            needTable = true;
            break;
        case CommandType::CheatEmptyDeck:
            round.leaveOneCardInDeck();
            ConsoleView::show("[cheat] The deck has only 1 card left.\n");
            needTable = true;
            break;
        case CommandType::CheatWinRound:
            if (round.cheatWinRound()) {
                ConsoleView::show("[cheat] The round ends in your favor.\n");
            } else {
                ConsoleView::show("[cheat] The round cannot be ended now.\n");
            }
            break;
        default:
            ConsoleView::show("That command is not available during a "
                              "game.\n");
            ConsoleView::show(ConsoleView::gameHelp());
            break;
    }

    return true;
}