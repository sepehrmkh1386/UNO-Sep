#include "CommandParser.h"

#include "../logic/TextUtils.h"

static Command makeCommand(CommandType type) {
    Command command;
    command.type = type;
    command.valid = true;
    return command;
}

static Command fail(CommandType type, const string& message) {
    Command command;
    command.type = type;
    command.valid = false;
    command.error = message;
    return command;
}

static bool parseColorName(const string& text, bool allowNone,
                           CardColor& color) {
    string upper = toUpperText(text);

    if (upper == "RED") {
        color = CardColor::Red;
        return true;
    }

    if (upper == "YELLOW") {
        color = CardColor::Yellow;
        return true;
    }

    if (upper == "GREEN") {
        color = CardColor::Green;
        return true;
    }

    if (upper == "BLUE") {
        color = CardColor::Blue;
        return true;
    }

    if (allowNone && upper == "NONE") {
        color = CardColor::None;
        return true;
    }

    return false;
}

static int optionIndex(const string& word, const string known[],
                       int knownCount) {
    string lower = toLowerText(word);

    for (int i = 0; i < knownCount; ++i) {
        if (lower == known[i]) {
            return i;
        }
    }

    return -1;
}

static bool readOptions(const DynamicArray<string>& words, int start,
                        const string known[], int knownCount,
                        bool found[], string values[], string& error) {
    int current = -1;

    for (int i = start; i < words.size(); ++i) {
        string word = wordAt(words, i);
        int option = optionIndex(word, known, knownCount);

        if (option >= 0) {
            if (found[option]) {
                error = "Option " + known[option] +
                        " is given more than once.";
                return false;
            }

            found[option] = true;
            values[option] = "";
            current = option;
        } else if (current >= 0) {
            if (!values[current].empty()) {
                values[current] += " ";
            }

            values[current] += word;
        } else {
            error = "Unexpected argument: " + word;
            return false;
        }
    }

    for (int i = 0; i < knownCount; ++i) {
        if (found[i] && values[i].empty()) {
            error = "Option " + known[i] + " needs a value.";
            return false;
        }
    }

    return true;
}

static Command noArguments(const DynamicArray<string>& words,
                           CommandType type, const string& name) {
    if (words.size() > 1) {
        return fail(type, "'" + name + "' does not take any arguments.");
    }

    return makeCommand(type);
}

static Command parseRegister(const DynamicArray<string>& words) {
    const string known[3] = {"-u", "-p", "-n"};
    bool found[3] = {false, false, false};
    string values[3];
    string error;

    if (!readOptions(words, 1, known, 3, found, values, error)) {
        return fail(CommandType::Register, error);
    }

    if (!found[0] || !found[1] || !found[2]) {
        return fail(CommandType::Register,
                    "Usage: register -u <username> -p <password> "
                    "-n <display_name>");
    }

    Command command = makeCommand(CommandType::Register);
    command.username = values[0];
    command.password = values[1];
    command.displayName = values[2];

    return command;
}

static Command parseLogin(const DynamicArray<string>& words) {
    const string known[2] = {"-u", "-p"};
    bool found[2] = {false, false};
    string values[2];
    string error;

    if (!readOptions(words, 1, known, 2, found, values, error)) {
        return fail(CommandType::Login, error);
    }

    if (!found[0] || !found[1]) {
        return fail(CommandType::Login,
                    "Usage: login -u <username> -p <password>");
    }

    Command command = makeCommand(CommandType::Login);
    command.username = values[0];
    command.password = values[1];

    return command;
}

static Command parseStartGame(const DynamicArray<string>& words) {
    const string known[4] = {"-p", "-b", "-mode", "-limit"};
    bool found[4] = {false, false, false, false};
    string values[4];
    string error;
    const string usage =
        "Usage: start_game -p <humans> -b <bots> "
        "-mode <score|rounds> -limit <N>";

    if (!readOptions(words, 1, known, 4, found, values, error)) {
        return fail(CommandType::StartGame, error);
    }

    if (!found[0] || !found[1] || !found[2] || !found[3]) {
        return fail(CommandType::StartGame, usage);
    }

    int humans = 0;
    int bots = 0;
    int limit = 0;

    if (!parseNonNegativeInt(values[0], humans) ||
        !parseNonNegativeInt(values[1], bots) ||
        !parseNonNegativeInt(values[3], limit)) {
        return fail(CommandType::StartGame,
                    "-p, -b and -limit must be whole numbers.");
    }

    string modeText = toLowerText(values[2]);
    GameMode mode = GameMode::Score;

    if (modeText == "score") {
        mode = GameMode::Score;
    } else if (modeText == "rounds") {
        mode = GameMode::Rounds;
    } else {
        return fail(CommandType::StartGame, "-mode must be score or rounds.");
    }

    if (humans < 1) {
        return fail(CommandType::StartGame,
                    "There must be at least 1 human player.");
    }

    if (humans + bots < 2 || humans + bots > 10) {
        return fail(CommandType::StartGame,
                    "The total number of players must be between 2 and 10.");
    }

    if (limit < 1) {
        return fail(CommandType::StartGame, "-limit must be at least 1.");
    }

    Command command = makeCommand(CommandType::StartGame);
    command.humans = humans;
    command.bots = bots;
    command.mode = mode;
    command.limit = limit;

    return command;
}

static Command parsePlay(const DynamicArray<string>& words) {
    if (words.size() < 2) {
        return fail(CommandType::Play,
                    "Usage: play <card_index> [-c <RED|YELLOW|GREEN|BLUE>]");
    }

    int index = 0;

    if (!parseNonNegativeInt(wordAt(words, 1), index)) {
        return fail(CommandType::Play, "The card index must be a number.");
    }

    const string known[1] = {"-c"};
    bool found[1] = {false};
    string values[1];
    string error;

    if (!readOptions(words, 2, known, 1, found, values, error)) {
        return fail(CommandType::Play, error);
    }

    Command command = makeCommand(CommandType::Play);
    command.cardIndex = index;

    if (found[0]) {
        if (!parseColorName(values[0], false, command.color)) {
            return fail(CommandType::Play,
                        "The color must be RED, YELLOW, GREEN or BLUE.");
        }

        command.hasColor = true;
    }

    return command;
}

static Command parsePlayDrawn(const DynamicArray<string>& words) {
    const string known[1] = {"-c"};
    bool found[1] = {false};
    string values[1];
    string error;

    if (!readOptions(words, 1, known, 1, found, values, error)) {
        return fail(CommandType::PlayDrawn, error);
    }

    Command command = makeCommand(CommandType::PlayDrawn);

    if (found[0]) {
        if (!parseColorName(values[0], false, command.color)) {
            return fail(CommandType::PlayDrawn,
                        "The color must be RED, YELLOW, GREEN or BLUE.");
        }

        command.hasColor = true;
    }

    return command;
}

static Command parseCheatCard(const DynamicArray<string>& words,
                              CommandType type, bool colorRequiredForWild) {
    const string known[3] = {"-t", "-c", "-v"};
    bool found[3] = {false, false, false};
    string values[3];
    string error;

    if (!readOptions(words, 1, known, 3, found, values, error)) {
        return fail(type, error);
    }

    if (!found[0]) {
        return fail(type,
                    "Usage: cheat -t <NUMBER|SKIP|REVERSE|DRAW_TWO|WILD|"
                    "WILD_DRAW_FOUR> -c <color> [-v <0-9>]");
    }

    CardColor color = CardColor::None;

    if (found[1] && !parseColorName(values[1], true, color)) {
        return fail(type,
                    "The color must be RED, YELLOW, GREEN, BLUE or NONE.");
    }

    int number = 0;

    if (found[2]) {
        if (!parseNonNegativeInt(values[2], number) || number > 9) {
            return fail(type, "-v must be a number from 0 to 9.");
        }
    }

    string cardType = toUpperText(values[0]);
    Command command = makeCommand(type);

    if (cardType == "NUMBER") {
        if (color == CardColor::None) {
            return fail(type, "NUMBER needs a real color (-c).");
        }

        if (!found[2]) {
            return fail(type, "NUMBER needs -v <0-9>.");
        }

        command.cheatCard = Card(color, number);
        return command;
    }

    CardType actionType = CardType::Skip;
    bool isWild = false;

    if (cardType == "SKIP") {
        actionType = CardType::Skip;
    } else if (cardType == "REVERSE") {
        actionType = CardType::Reverse;
    } else if (cardType == "DRAW_TWO") {
        actionType = CardType::DrawTwo;
    } else if (cardType == "WILD") {
        actionType = CardType::Wild;
        isWild = true;
    } else if (cardType == "WILD_DRAW_FOUR") {
        actionType = CardType::WildDrawFour;
        isWild = true;
    } else {
        return fail(type, "Unknown card type: " + values[0]);
    }

    if (found[2]) {
        return fail(type, "-v is only used with NUMBER.");
    }

    if (!isWild) {
        if (color == CardColor::None) {
            return fail(type, "This card type needs a real color (-c).");
        }

        command.cheatCard = Card(color, actionType);
        return command;
    }

    if (colorRequiredForWild && color == CardColor::None) {
        return fail(type, "A wild card on the table needs a color (-c).");
    }

    Card card(CardColor::None, actionType);

    if (color != CardColor::None) {
        card.setColor(color);
    }

    command.cheatCard = card;
    return command;
}

Command CommandParser::parse(const string& line) {
    DynamicArray<string> words;
    splitWords(line, words);

    if (words.size() == 0) {
        return Command();
    }

    string name = toLowerText(wordAt(words, 0));

    if (name == "register") {
        return parseRegister(words);
    }

    if (name == "login") {
        return parseLogin(words);
    }

    if (name == "start_game") {
        return parseStartGame(words);
    }

    if (name == "play") {
        return parsePlay(words);
    }

    if (name == "play_drawn") {
        return parsePlayDrawn(words);
    }

    if (name == "cheat_give") {
        return parseCheatCard(words, CommandType::CheatGive, false);
    }

    if (name == "cheat_top") {
        return parseCheatCard(words, CommandType::CheatTop, true);
    }

    if (name == "leaderboard") {
        return noArguments(words, CommandType::Leaderboard, name);
    }

    if (name == "logout" || name == "exit") {
        return noArguments(words, CommandType::Logout, name);
    }

    if (name == "draw") {
        return noArguments(words, CommandType::Draw, name);
    }

    if (name == "pass") {
        return noArguments(words, CommandType::Pass, name);
    }

    if (name == "chance") {
        return noArguments(words, CommandType::Chance, name);
    }

    if (name == "scores") {
        return noArguments(words, CommandType::Scores, name);
    }

    if (name == "quit_game") {
        return noArguments(words, CommandType::QuitGame, name);
    }

    if (name == "cheat_empty_deck") {
        return noArguments(words, CommandType::CheatEmptyDeck, name);
    }

    if (name == "cheat_win_round") {
        return noArguments(words, CommandType::CheatWinRound, name);
    }

    return fail(CommandType::Unknown, "Unknown command: " + name);
}