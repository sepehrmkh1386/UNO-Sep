#include <iostream>
#include <string>

#include "../src/logic/TextUtils.h"
#include "../src/model/BotPlayer.h"
#include "../src/model/HumanPlayer.h"
#include "../src/view/CommandParser.h"
#include "../src/view/ConsoleView.h"

using namespace std;

static int failedChecks = 0;

static void check(bool condition, const char* name) {
    if (condition) {
        cout << "[PASS] " << name << '\n';
    } else {
        cout << "[FAIL] " << name << '\n';
        ++failedChecks;
    }
}

static bool has(const string& text, const string& part) {
    return text.find(part) != string::npos;
}

static bool ok(const string& line, CommandType type) {
    Command command = CommandParser::parse(line);
    return command.valid && command.type == type;
}

static bool bad(const string& line) {
    return !CommandParser::parse(line).valid;
}

static void testTextUtils() {
    check(toLowerText("AbC-1") == "abc-1" && toUpperText("aBc-1") == "ABC-1",
          "text: case conversion");

    int number = 0;
    check(parseNonNegativeInt("12", number) && number == 12 &&
          parseNonNegativeInt("0", number) && number == 0,
          "text: valid numbers are parsed");
    check(!parseNonNegativeInt("", number) && !parseNonNegativeInt("a1", number) &&
          !parseNonNegativeInt("-1", number) && !parseNonNegativeInt("1.5", number) &&
          !parseNonNegativeInt("1234567890", number),
          "text: invalid numbers are rejected");

    DynamicArray<string> parts;
    splitBy("a|b||c", '|', parts);
    check(parts.size() == 4 && wordAt(parts, 2) == "" && wordAt(parts, 3) == "c",
          "text: splitBy keeps empty fields");

    DynamicArray<string> words;
    splitWords("  play   3  -c  red ", words);
    check(words.size() == 4 && wordAt(words, 0) == "play" &&
          wordAt(words, 3) == "red",
          "text: splitWords ignores extra spaces");

    check(padRight("ab", 5) == "ab   " && padRight("abcdef", 3) == "abcdef",
          "text: padRight pads but never cuts");
    check(hasWhitespace("a b") && !hasWhitespace("ab") &&
          containsChar("a|b", '|') && !containsChar("ab", '|'),
          "text: hasWhitespace and containsChar");
}

static void testAuthCommands() {
    check(CommandParser::parse("").type == CommandType::Empty &&
          CommandParser::parse("   ").type == CommandType::Empty,
          "parse: empty lines");

    Command unknown = CommandParser::parse("foo");
    check(!unknown.valid && unknown.type == CommandType::Unknown &&
          has(unknown.error, "foo"),
          "parse: an unknown command is reported");

    Command registered =
        CommandParser::parse("register -u ali -p 1234 -n Ali Rezaei");
    check(registered.valid && registered.type == CommandType::Register &&
          registered.username == "ali" && registered.password == "1234" &&
          registered.displayName == "Ali Rezaei",
          "register: fields and a multi-word display name");
    check(ok("register -n Ali -p 1234 -u ali", CommandType::Register),
          "register: the option order does not matter");
    check(ok("REGISTER -U ali -P 1234 -N Ali", CommandType::Register),
          "register: commands and options are case-insensitive");
    check(bad("register -u ali -p 1234") && bad("register -u ali -p -n Ali") &&
          bad("register -u ali -u bob -p 1234 -n A") &&
          bad("register ali 1234 Ali") && bad("register"),
          "register: missing, empty, repeated or stray options are rejected");

    Command login = CommandParser::parse("login -u ali -p 1234");
    check(login.valid && login.type == CommandType::Login &&
          login.username == "ali" && login.password == "1234",
          "login: fields");
    check(bad("login -u ali") && bad("login -p 1234"),
          "login: both options are required");

    check(ok("leaderboard", CommandType::Leaderboard) &&
          bad("leaderboard extra"),
          "leaderboard: no arguments");
    check(ok("logout", CommandType::Logout) && ok("exit", CommandType::Logout) &&
          bad("exit now"),
          "logout and exit: the same command, no arguments");
}

static void testStartGame() {
    Command score = CommandParser::parse(
        "start_game -p 1 -b 3 -mode score -limit 500");
    check(score.valid && score.type == CommandType::StartGame &&
          score.humans == 1 && score.bots == 3 &&
          score.mode == GameMode::Score && score.limit == 500,
          "start_game: score mode");

    Command rounds = CommandParser::parse(
        "start_game -p 2 -b 0 -mode ROUNDS -limit 3");
    check(rounds.valid && rounds.humans == 2 && rounds.bots == 0 &&
          rounds.mode == GameMode::Rounds && rounds.limit == 3,
          "start_game: rounds mode, 2 humans, no bots");

    check(bad("start_game -p 1 -b 3 -mode fast -limit 500"),
          "start_game: an unknown mode is rejected");
    check(bad("start_game -p 0 -b 3 -mode score -limit 500"),
          "start_game: at least 1 human is needed");
    check(bad("start_game -p 1 -b 0 -mode score -limit 500"),
          "start_game: 1 player in total is rejected");
    check(bad("start_game -p 5 -b 6 -mode score -limit 500") &&
          ok("start_game -p 5 -b 5 -mode score -limit 500",
             CommandType::StartGame),
          "start_game: 11 players are rejected, 10 are fine");
    check(bad("start_game -p 1 -b 3 -mode score -limit 0") &&
          bad("start_game -p 1 -b 3 -mode score -limit abc") &&
          bad("start_game -p -1 -b 3 -mode score -limit 5") &&
          bad("start_game -p 1 -b 3 -mode score") &&
          bad("start_game"),
          "start_game: bad or missing numbers are rejected");
}

static void testPlayCommands() {
    Command plain = CommandParser::parse("play 1");
    check(plain.valid && plain.type == CommandType::Play &&
          plain.cardIndex == 1 && !plain.hasColor,
          "play: index only");

    Command colored = CommandParser::parse("play 2 -c blue");
    check(colored.valid && colored.cardIndex == 2 && colored.hasColor &&
          colored.color == CardColor::Blue,
          "play: a lowercase color is accepted");

    check(bad("play") && bad("play x") && bad("play -1") &&
          bad("play 1 -c pink") && bad("play 1 -c") && bad("play 1 -c none") &&
          bad("play 1 -t 2"),
          "play: bad indices, colors and options are rejected");

    check(ok("draw", CommandType::Draw) && ok("pass", CommandType::Pass) &&
          ok("chance", CommandType::Chance) && ok("scores", CommandType::Scores) &&
          ok("quit_game", CommandType::QuitGame),
          "simple game commands");
    check(bad("draw 3") && bad("pass now") && bad("chance 1"),
          "simple game commands take no arguments");

    Command drawn = CommandParser::parse("play_drawn");
    check(drawn.valid && drawn.type == CommandType::PlayDrawn && !drawn.hasColor,
          "play_drawn: no color");

    Command drawnColor = CommandParser::parse("play_drawn -c red");
    check(drawnColor.valid && drawnColor.hasColor &&
          drawnColor.color == CardColor::Red,
          "play_drawn: with a color");
    check(bad("play_drawn -c none") && bad("play_drawn -c") &&
          bad("play_drawn 1"),
          "play_drawn: bad options are rejected");
}

static void testCheatCommands() {
    Command wild = CommandParser::parse("cheat_give -t WILD_DRAW_FOUR -c NONE");
    check(wild.valid && wild.type == CommandType::CheatGive &&
          wild.cheatCard.getType() == CardType::WildDrawFour &&
          wild.cheatCard.getColor() == CardColor::None,
          "cheat_give: wild draw four without a color");

    Command number = CommandParser::parse("cheat_give -t NUMBER -c RED -v 7");
    check(number.valid && number.cheatCard.isNumberCard() &&
          number.cheatCard.getColor() == CardColor::Red &&
          number.cheatCard.getValue() == 7,
          "cheat_give: a red 7");

    Command skip = CommandParser::parse("cheat_give -t skip -c blue");
    check(skip.valid && skip.cheatCard.getType() == CardType::Skip &&
          skip.cheatCard.getColor() == CardColor::Blue &&
          skip.cheatCard.getValue() == 20,
          "cheat_give: a blue skip (case-insensitive)");

    check(ok("cheat_give -t WILD", CommandType::CheatGive) &&
          ok("cheat_give -t DRAW_TWO -c GREEN", CommandType::CheatGive) &&
          ok("cheat_give -t REVERSE -c YELLOW", CommandType::CheatGive) &&
          ok("cheat_give -t NUMBER -c YELLOW -v 0", CommandType::CheatGive),
          "cheat_give: every card type can be created");

    check(bad("cheat_give") && bad("cheat_give -t BANANA -c RED") &&
          bad("cheat_give -t NUMBER -c RED") &&
          bad("cheat_give -t NUMBER -c NONE -v 7") &&
          bad("cheat_give -t NUMBER -c RED -v 10") &&
          bad("cheat_give -t SKIP") &&
          bad("cheat_give -t SKIP -c RED -v 3") &&
          bad("cheat_give -t WILD -v 3") &&
          bad("cheat_give -t WILD -c PINK"),
          "cheat_give: invalid combinations are rejected");

    Command top = CommandParser::parse("cheat_top -t WILD -c GREEN");
    check(top.valid && top.type == CommandType::CheatTop &&
          top.cheatCard.getType() == CardType::Wild &&
          top.cheatCard.getColor() == CardColor::Green,
          "cheat_top: a wild card with the active color");
    check(ok("cheat_top -t NUMBER -c YELLOW -v 0", CommandType::CheatTop) &&
          bad("cheat_top -t WILD") && bad("cheat_top -t WILD_DRAW_FOUR"),
          "cheat_top: a wild card needs a color");

    check(ok("cheat_empty_deck", CommandType::CheatEmptyDeck) &&
          ok("cheat_win_round", CommandType::CheatWinRound) &&
          bad("cheat_empty_deck 1"),
          "cheat_empty_deck and cheat_win_round");
}

static void testHandAndTable() {
    HumanPlayer a("A");
    HumanPlayer b("B");
    HumanPlayer c("C");
    DynamicArray<Player*> seats;
    seats.add(&a);
    seats.add(&b);
    seats.add(&c);

    HumanPlayer lone("Lone");
    check(has(ConsoleView::formatHand(lone), "no cards"),
          "hand: an empty hand is described");

    lone.addCard(Card(CardColor::Red, 7));
    lone.addCard(Card(CardColor::None, CardType::WildDrawFour));
    lone.addCard(Card(CardColor::Blue, CardType::Skip));
    lone.addCard(Card(CardColor::Green, 1));
    string handText = ConsoleView::formatHand(lone);
    check(has(handText, "[0] RED 7") && has(handText, "[1] WILD DRAW FOUR (+4)") &&
          has(handText, "[2] BLUE SKIP") && has(handText, "[3] GREEN 1"),
          "hand: every card is listed with its index");

    Game game(seats, GameMode::Score, 500);
    game.start();
    string table = ConsoleView::formatTable(game, *game.getRound());

    check(has(table, "ROUND #1") && has(table, "Direction: ") &&
          has(table, "Win Condition: 500 Pts"),
          "table: round number, direction and win condition");
    check(has(table, "Deck Remaining: ") &&
          has(table, "Pending Draw Penalty: 0") &&
          has(table, "Top Card on Table: [") && has(table, "Active Color: "),
          "table: deck, penalty, top card and active color");
    check(has(table, "(You)*: ") && has(table, " --> "),
          "table: the current player is marked in the players line");
    check(has(table, "Your Hand (") && has(table, "[0] "),
          "table: a human sees their own hand");

    a.clearHand();
    b.clearHand();
    c.clearHand();
    a.addCard(Card(CardColor::Red, 1));
    b.addCard(Card(CardColor::Red, 2));
    c.addCard(Card(CardColor::Red, 3));
    string singular = ConsoleView::formatTable(game, *game.getRound());
    check(has(singular, ": 1 card]") && !has(singular, "1 cards"),
          "table: one card is written in the singular");

    Game rounds(seats, GameMode::Rounds, 3);
    rounds.start();
    check(has(ConsoleView::formatTable(rounds, *rounds.getRound()),
              "Win Condition: 3 Rounds"),
          "table: rounds mode shows the number of rounds");

    DynamicArray<Player*> botSeats;
    BotPlayer botA("Bot_1");
    BotPlayer botB("Bot_2");
    botSeats.add(&botA);
    botSeats.add(&botB);
    Game botGame(botSeats, GameMode::Score, 100);
    botGame.start();
    string botTable = ConsoleView::formatTable(botGame, *botGame.getRound());
    check(!has(botTable, "Your Hand") && has(botTable, "*Bot_"),
          "table: no hand is shown when a bot is the current player");
}

static void playRoundToEnd(Game& game) {
    Round* round = game.getRound();
    int steps = 0;

    while (!round->isOver() && steps < 20000) {
        Action action;
        MoveResult result;

        if (!game.stepBot(action, result)) {
            break;
        }

        ++steps;
    }
}

static void testBotMoveText() {
    BotPlayer bot("Bot_1");
    Round* none = nullptr;
    (void)none;

    DynamicArray<Player*> seats;
    BotPlayer b1("Bot_1");
    BotPlayer b2("Bot_2");
    BotPlayer b3("Bot_3");
    seats.add(&b1);
    seats.add(&b2);
    seats.add(&b3);

    Game game(seats, GameMode::Score, 1000);
    game.start();
    Round* round = game.getRound();

    check(ConsoleView::formatBotMove(bot, Action(ActionType::Pass), *round) ==
              "Bot_1 passes",
          "bot move: pass");
    check(has(ConsoleView::formatBotMove(
                  bot, Action(ActionType::None, -1, CardColor::Green), *round),
              "starting color: GREEN"),
          "bot move: choosing the starting color");
    check(has(ConsoleView::formatBotMove(bot, Action(ActionType::Draw), *round),
              "Bot_1 draws "),
          "bot move: draw");

    int checked = 0;
    bool allNamed = true;

    for (int i = 0; i < 40 && !round->isOver(); ++i) {
        Player* mover = round->getCurrentPlayer();
        Action action;
        MoveResult result;

        if (!game.stepBot(action, result)) {
            break;
        }

        string text = ConsoleView::formatBotMove(*mover, action, *round);

        if (text.find(mover->getName()) != 0) {
            allNamed = false;
        }

        ++checked;
    }

    check(checked > 0 && allNamed,
          "bot move: every real move starts with the bot's name");
}

static void testSummaryAndScores() {
    DynamicArray<Player*> seats;
    BotPlayer b1("Bot_1");
    BotPlayer b2("Bot_2");
    BotPlayer b3("Bot_3");
    seats.add(&b1);
    seats.add(&b2);
    seats.add(&b3);

    Game game(seats, GameMode::Score, 1);
    game.start();

    check(ConsoleView::formatRoundSummary(game) == "",
          "summary: nothing is printed while the round is running");

    playRoundToEnd(game);
    game.finishRound();

    string summary = ConsoleView::formatRoundSummary(game);
    string winnerName = game.getRound()->getWinner()->getName();

    check(has(summary, "ROUND OVER") && has(summary, "Winner: " + winnerName),
          "summary: the winner is named");
    check(has(summary, "Remaining cards:") && has(summary, " pts") &&
          has(summary, "Points added to " + winnerName),
          "summary: remaining cards, points and the added score");
    check(has(summary, "--- Scores after 1 round ---") &&
          has(summary, "Total: ") && has(summary, "R1="),
          "summary: the scoreboard is included");

    if (game.isFinished()) {
        check(has(summary, "GAME OVER! Winner: "),
              "summary: the end of the game is announced");
    } else {
        check(!has(summary, "GAME OVER"),
              "summary: no game over text while the game continues");
    }

    check(has(ConsoleView::formatScores(game), "Bot_2"),
          "scores: every player is listed");
}

static void testLeaderboardAndHelp() {
    DynamicArray<Account*> empty;
    check(has(ConsoleView::formatLeaderboard(empty), "No registered users."),
          "leaderboard: an empty list is described");

    Account first("ali", "1234", "Ali");
    Account second("sara", "abcd", "Sara");
    first.recordGame(120, true);
    second.recordGame(40, false);

    DynamicArray<Account*> ranked;
    ranked.add(&first);
    ranked.add(&second);
    string text = ConsoleView::formatLeaderboard(ranked);

    check(has(text, "Rank") && has(text, "Ali") && has(text, "sara") &&
          has(text, "120") && text.find("Ali") < text.find("Sara"),
          "leaderboard: rank, names, scores and order");

    check(ConsoleView::describeResult(MoveResult::Ok) == "" &&
          ConsoleView::describeResult(MoveResult::InvalidIndex) != "" &&
          ConsoleView::describeResult(MoveResult::InvalidMove) != "" &&
          ConsoleView::describeResult(MoveResult::ColorRequired) != "" &&
          ConsoleView::describeResult(MoveResult::AwaitingStartColor) != "" &&
          ConsoleView::describeResult(MoveResult::RoundOver) != "" &&
          ConsoleView::describeResult(MoveResult::NotAllowed) != "",
          "messages: every move result has a text");

    check(ConsoleView::describeRegisterResult(RegisterResult::Ok) == "" &&
          ConsoleView::describeRegisterResult(RegisterResult::UsernameTaken) != "" &&
          ConsoleView::describeRegisterResult(RegisterResult::WeakPassword) != "" &&
          ConsoleView::describeRegisterResult(RegisterResult::InvalidUsername) != "" &&
          ConsoleView::describeRegisterResult(RegisterResult::InvalidDisplayName) != "" &&
          ConsoleView::describeRegisterResult(RegisterResult::ForbiddenCharacter) != "",
          "messages: every register result has a text");

    check(has(ConsoleView::authHelp(), "register -u") &&
          has(ConsoleView::mainHelp(), "start_game -p") &&
          has(ConsoleView::gameHelp(), "play_drawn"),
          "help: the three menus describe their commands");
}

int main() {
    cout << "--- Text utilities ---\n";
    testTextUtils();

    cout << "\n--- Register, login, leaderboard commands ---\n";
    testAuthCommands();

    cout << "\n--- start_game command ---\n";
    testStartGame();

    cout << "\n--- Play commands ---\n";
    testPlayCommands();

    cout << "\n--- Cheat commands ---\n";
    testCheatCommands();

    cout << "\n--- Hand and table ---\n";
    testHandAndTable();

    cout << "\n--- Bot move text ---\n";
    testBotMoveText();

    cout << "\n--- Round summary and scores ---\n";
    testSummaryAndScores();

    cout << "\n--- Leaderboard, messages and help ---\n";
    testLeaderboardAndHelp();

    cout << "\nFailed checks: " << failedChecks << '\n';
    return failedChecks == 0 ? 0 : 1;
}