#include "ConsoleView.h"

#include <iostream>

#include "../logic/TextUtils.h"

static string cardCountText(int count) {
    if (count == 1) {
        return "1 card";
    }

    return to_string(count) + " cards";
}

static string winConditionText(const Game& game) {
    if (game.getMode() == GameMode::Score) {
        return to_string(game.getLimit()) + " Pts";
    }

    return to_string(game.getLimit()) + " Rounds";
}

static string playersLine(const Round& round) {
    string text = "";
    Player* current = round.getCurrentPlayer();

    for (int i = 0; i < round.getPlayerCount(); ++i) {
        Player* player = round.getPlayerAt(i);
        string label = player->getName();

        if (player == current) {
            if (player->isHuman()) {
                label = "*" + label + " (You)*";
            } else {
                label = "*" + label + "*";
            }
        }

        if (i > 0) {
            text += " --> ";
        }

        text += "[" + label + ": " + cardCountText(player->getHandSize()) + "]";
    }

    return text;
}

static string handAsList(const Player& player) {
    string text = "";

    for (int i = 0; i < player.getHandSize(); ++i) {
        Card card;
        player.getCard(i, card);

        if (i > 0) {
            text += ", ";
        }

        text += card.toString();
    }

    if (text.empty()) {
        return "(none)";
    }

    return text;
}

string ConsoleView::formatHand(const Player& player) {
    if (player.getHandSize() == 0) {
        return "  (no cards)\n";
    }

    string text = "";

    for (int i = 0; i < player.getHandSize(); ++i) {
        Card card;
        player.getCard(i, card);

        string entry = "[" + to_string(i) + "] " + card.toString();
        text += "  " + padRight(entry, 26);

        if ((i + 1) % 3 == 0 || i == player.getHandSize() - 1) {
            text += "\n";
        }
    }

    return text;
}

string ConsoleView::formatTable(const Game& game, const Round& round) {
    string bar(64, '=');
    string thin(64, '-');
    string text = "";

    text += bar + "\n";
    text += "ROUND #" + to_string(game.getRoundsPlayed() + 1);
    text += "  |  Direction: ";
    text += round.isClockwise() ? "CLOCKWISE (-->)" : "COUNTER-CLOCKWISE (<--)";
    text += "  |  Win Condition: " + winConditionText(game) + "\n";
    text += "Players:  " + playersLine(round) + "\n";
    text += thin + "\n";

    text += "Deck Remaining: " + to_string(round.getDeckSize()) + " cards";
    text += "   |   Pending Draw Penalty: ";
    text += to_string(round.getPendingPenalty()) + "\n";

    Card top;
    round.getTopCard(top);
    text += "Top Card on Table: [" + top.toString() + "]";
    text += "   |   Active Color: " + colorToString(round.getActiveColor()) + "\n";
    text += thin + "\n";

    Player* current = round.getCurrentPlayer();

    if (current != nullptr && current->isHuman()) {
        text += "Your Hand (" + current->getName() + "):\n";
        text += formatHand(*current);

        if (round.hasDrawnCard()) {
            text += "You drew a playable card: use play_drawn [-c <color>] "
                    "or pass\n";
        }
    }

    text += bar + "\n";

    return text;
}

string ConsoleView::formatScores(const Game& game) {
    string text = "--- Scores after " + to_string(game.getRoundsPlayed());
    text += game.getRoundsPlayed() == 1 ? " round ---\n" : " rounds ---\n";

    for (int i = 0; i < game.getPlayerCount(); ++i) {
        Player* player = game.getPlayerAt(i);

        text += padRight(player->getName(), 14);
        text += "Total: " + padRight(to_string(player->getCurrentGameScore()), 7);

        Node<int>* node = player->getRoundScores().getHead();
        int roundNumber = 1;

        while (node != nullptr) {
            text += "R" + to_string(roundNumber) + "=" +
                    to_string(node->data) + " ";
            node = node->next;
            ++roundNumber;
        }

        text += "\n";
    }

    return text;
}

string ConsoleView::formatRoundSummary(const Game& game) {
    Round* round = game.getRound();

    if (round == nullptr || !round->isOver() || round->getWinner() == nullptr) {
        return "";
    }

    Player* winner = round->getWinner();
    string text = "";

    text += "=============== ROUND OVER ===============\n";
    text += "Winner: " + winner->getName() + "\n";
    text += "Remaining cards:\n";

    for (int i = 0; i < game.getPlayerCount(); ++i) {
        Player* player = game.getPlayerAt(i);

        if (player == winner) {
            continue;
        }

        text += "  " + player->getName() + ": " + handAsList(*player);
        text += "  =>  " + to_string(player->handScore()) + " pts\n";
    }

    text += "Points added to " + winner->getName() + ": ";
    text += to_string(game.getLastRoundPoints()) + "\n";
    text += formatScores(game);

    if (game.isFinished() && game.getGameWinner() != nullptr) {
        text += "GAME OVER! Winner: " + game.getGameWinner()->getName() + "\n";
    }

    return text;
}

string ConsoleView::formatLeaderboard(const DynamicArray<Account*>& ranked) {
    string text = "============= LEADERBOARD =============\n";

    if (ranked.size() == 0) {
        return text + "No registered users.\n";
    }

    text += padRight("Rank", 6) + padRight("Name", 16) +
            padRight("Username", 16) + padRight("Score", 8) +
            padRight("Games", 7) + "Wins\n";

    for (int i = 0; i < ranked.size(); ++i) {
        Account* account = nullptr;
        ranked.get(i, account);

        text += padRight(to_string(i + 1), 6);
        text += padRight(account->getDisplayName(), 16);
        text += padRight(account->getUsername(), 16);
        text += padRight(to_string(account->getTotalScore()), 8);
        text += padRight(to_string(account->getGamesPlayed()), 7);
        text += to_string(account->getWins()) + "\n";
    }

    return text;
}

string ConsoleView::formatBotMove(const Player& bot, const Action& action,
                                  const Round& round) {
    string name = bot.getName();
    Card top;
    round.getTopCard(top);

    switch (action.type) {
        case ActionType::Play:
        case ActionType::PlayDrawn: {
            string text = name + " plays " + top.toString();

            if (top.isWildCard()) {
                text += " (color: " + colorToString(round.getActiveColor()) + ")";
            }

            return text;
        }
        case ActionType::Draw:
            return name + " draws " + cardCountText(round.getLastDrawCount());
        case ActionType::Chance:
            return name + " uses the chance machine and draws " +
                   cardCountText(round.getLastDrawCount());
        case ActionType::Pass:
            return name + " passes";
        case ActionType::None:
            if (action.color != CardColor::None) {
                return name + " chooses the starting color: " +
                       colorToString(action.color);
            }

            return name + " waits";
    }

    return name + " waits";
}

string ConsoleView::describeResult(MoveResult result) {
    switch (result) {
        case MoveResult::Ok:
            return "";
        case MoveResult::InvalidIndex:
            return "Invalid card index.";
        case MoveResult::InvalidMove:
            return "You cannot play that card now.";
        case MoveResult::ColorRequired:
            return "Choose a color: play <card_index> -c "
                   "<RED|YELLOW|GREEN|BLUE>";
        case MoveResult::AwaitingStartColor:
            return "The starting color has not been chosen yet.";
        case MoveResult::RoundOver:
            return "The round is over.";
        case MoveResult::NotAllowed:
            return "That command is not allowed right now.";
    }

    return "";
}

string ConsoleView::describeRegisterResult(RegisterResult result) {
    switch (result) {
        case RegisterResult::Ok:
            return "";
        case RegisterResult::InvalidUsername:
            return "The username cannot be empty or contain spaces.";
        case RegisterResult::UsernameTaken:
            return "This username is already taken.";
        case RegisterResult::WeakPassword:
            return "The password must have at least 4 characters and no spaces.";
        case RegisterResult::InvalidDisplayName:
            return "The display name cannot be empty.";
        case RegisterResult::ForbiddenCharacter:
            return "The character '|' is not allowed.";
    }

    return "";
}

string ConsoleView::authHelp() {
    return "Commands:\n"
           "  register -u <username> -p <password> -n <display_name>\n"
           "  login -u <username> -p <password>\n"
           "  leaderboard\n"
           "  exit\n";
}

string ConsoleView::mainHelp() {
    return "Commands:\n"
           "  start_game -p <humans> -b <bots> -mode <score|rounds> -limit <N>\n"
           "  leaderboard\n"
           "  logout\n";
}

string ConsoleView::gameHelp() {
    return "Commands:\n"
           "  play <card_index>\n"
           "  play <card_index> -c <RED|YELLOW|GREEN|BLUE>   (wild cards)\n"
           "  draw\n"
           "  play_drawn [-c <color>]  |  pass\n"
           "  chance   (only while a draw penalty is pending)\n"
           "  scores  |  quit_game\n";
}

void ConsoleView::show(const string& text) {
    cout << text;
}

bool ConsoleView::readLine(const string& prompt, string& line) {
    cout << prompt;

    if (!getline(cin, line)) {
        return false;
    }

    return true;
}