#include <iostream>

#include "../src/model/BotPlayer.h"

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

static Card num(CardColor color, int number) {
    return Card(color, number);
}

static Card act(CardColor color, CardType type) {
    return Card(color, type);
}

static Card wild() {
    return Card(CardColor::None, CardType::Wild);
}

static Card wildDrawFour() {
    return Card(CardColor::None, CardType::WildDrawFour);
}

static void setHand(BotPlayer& bot, const Card cards[], int count) {
    bot.clearHand();

    for (int i = 0; i < count; ++i) {
        bot.addCard(cards[i]);
    }
}

static RoundState makeState(const Card& top, CardColor active, int penalty,
                            bool hasDrawn) {
    RoundState state;
    state.topCard = top;
    state.activeColor = active;
    state.pendingPenalty = penalty;
    state.hasDrawn = hasDrawn;
    return state;
}

static bool is(const Action& action, ActionType type, int index,
               CardColor color) {
    return action.type == type && action.cardIndex == index &&
           action.color == color;
}

static void testBestColor() {
    BotPlayer bot("Bot_1");

    check(bot.chooseBestColor() == CardColor::Red,
          "color: empty hand defaults to RED");

    const Card blueMajority[] = {num(CardColor::Blue, 1),
                                 num(CardColor::Blue, 2),
                                 num(CardColor::Green, 3)};
    setHand(bot, blueMajority, 3);
    check(bot.chooseBestColor() == CardColor::Blue,
          "color: the most frequent color wins (blue)");

    const Card greenMajority[] = {num(CardColor::Green, 1),
                                  num(CardColor::Green, 2),
                                  num(CardColor::Blue, 3),
                                  num(CardColor::Red, 4)};
    setHand(bot, greenMajority, 4);
    check(bot.chooseBestColor() == CardColor::Green,
          "color: the most frequent color wins (green)");

    const Card yellowMajority[] = {num(CardColor::Yellow, 1),
                                   num(CardColor::Yellow, 2),
                                   num(CardColor::Red, 3)};
    setHand(bot, yellowMajority, 3);
    check(bot.chooseBestColor() == CardColor::Yellow,
          "color: the most frequent color wins (yellow)");

    const Card greenBlue[] = {num(CardColor::Green, 3), num(CardColor::Blue, 4)};
    setHand(bot, greenBlue, 2);
    check(bot.chooseBestColor() == CardColor::Blue,
          "color: tie between blue and green picks blue (RED, BLUE, GREEN, YELLOW)");

    const Card yellowGreen[] = {num(CardColor::Yellow, 1),
                                num(CardColor::Green, 2)};
    setHand(bot, yellowGreen, 2);
    check(bot.chooseBestColor() == CardColor::Green,
          "color: tie between green and yellow picks green");

    const Card yellowRed[] = {num(CardColor::Yellow, 1), num(CardColor::Red, 2)};
    setHand(bot, yellowRed, 2);
    check(bot.chooseBestColor() == CardColor::Red,
          "color: tie between red and yellow picks red");

    const Card onlyWilds[] = {wild(), wildDrawFour()};
    setHand(bot, onlyWilds, 2);
    check(bot.chooseBestColor() == CardColor::Red,
          "color: a hand with only wilds defaults to RED");

    const Card wildsIgnored[] = {wild(), wild(), wildDrawFour(),
                                 num(CardColor::Yellow, 1)};
    setHand(bot, wildsIgnored, 4);
    check(bot.chooseBestColor() == CardColor::Yellow,
          "color: wild cards are not counted as a color");
}

static void testPenaltyMode() {
    BotPlayer bot("Bot_1");
    RoundState state = makeState(act(CardColor::Red, CardType::DrawTwo),
                                 CardColor::Red, 2, false);

    const Card firstDraw[] = {num(CardColor::Red, 3),
                              act(CardColor::Blue, CardType::DrawTwo),
                              wildDrawFour(),
                              act(CardColor::Green, CardType::DrawTwo)};
    setHand(bot, firstDraw, 4);
    check(is(bot.chooseAction(state), ActionType::Play, 1, CardColor::None),
          "penalty: plays the first Draw card found from index 0");

    const Card plusFourFirst[] = {num(CardColor::Blue, 1),
                                  num(CardColor::Blue, 2), wildDrawFour(),
                                  num(CardColor::Green, 3)};
    setHand(bot, plusFourFirst, 4);
    check(is(bot.chooseAction(state), ActionType::Play, 2, CardColor::Blue),
          "penalty: WildDrawFour is played with the best color");

    const Card noDraw[] = {num(CardColor::Red, 3),
                           act(CardColor::Red, CardType::Skip), wild()};
    setHand(bot, noDraw, 3);
    check(is(bot.chooseAction(state), ActionType::Draw, -1, CardColor::None),
          "penalty: draws the penalty when there is no Draw card");

    bot.clearHand();
    check(bot.chooseAction(state).type == ActionType::Draw,
          "penalty: an empty hand draws");
}

static void testNormalMode() {
    BotPlayer bot("Bot_1");
    RoundState state = makeState(num(CardColor::Red, 5), CardColor::Red, 0,
                                 false);

    const Card coloredFirst[] = {wild(), num(CardColor::Green, 7),
                                 num(CardColor::Red, 2)};
    setHand(bot, coloredFirst, 3);
    check(is(bot.chooseAction(state), ActionType::Play, 2, CardColor::None),
          "normal: a valid colored card beats a wild card");

    const Card firstValid[] = {num(CardColor::Blue, 5), num(CardColor::Red, 2)};
    setHand(bot, firstValid, 2);
    check(is(bot.chooseAction(state), ActionType::Play, 0, CardColor::None),
          "normal: the first valid card in hand order is chosen");

    const Card wildSecond[] = {num(CardColor::Green, 7), wild(),
                               wildDrawFour()};
    setHand(bot, wildSecond, 3);
    check(is(bot.chooseAction(state), ActionType::Play, 1, CardColor::Green),
          "normal: wild is used when no colored card is valid (best color)");

    const Card plusFourOnly[] = {num(CardColor::Green, 7), wildDrawFour()};
    setHand(bot, plusFourOnly, 2);
    check(is(bot.chooseAction(state), ActionType::Play, 1, CardColor::Green),
          "normal: WildDrawFour is the last resort when no red card is held");

    const Card plusFourBlocked[] = {wildDrawFour(), num(CardColor::Red, 9)};
    setHand(bot, plusFourBlocked, 2);
    check(is(bot.chooseAction(state), ActionType::Play, 1, CardColor::None),
          "normal: WildDrawFour is never played while an active-color card exists");

    const Card nothing[] = {num(CardColor::Green, 7), num(CardColor::Blue, 9)};
    setHand(bot, nothing, 2);
    check(is(bot.chooseAction(state), ActionType::Draw, -1, CardColor::None),
          "normal: draws when no card is playable");

    bot.clearHand();
    check(bot.chooseAction(state).type == ActionType::Draw,
          "normal: an empty hand draws");

    RoundState drawTwoTop = makeState(act(CardColor::Blue, CardType::DrawTwo),
                                      CardColor::Blue, 0, false);
    const Card sameType[] = {num(CardColor::Green, 1),
                             act(CardColor::Green, CardType::DrawTwo)};
    setHand(bot, sameType, 2);
    check(is(bot.chooseAction(drawTwoTop), ActionType::Play, 1, CardColor::None),
          "normal: the same action type matches even with another color");

    RoundState wildTop = makeState(wild(), CardColor::Green, 0, false);
    const Card afterWild[] = {num(CardColor::Red, 1), num(CardColor::Green, 3)};
    setHand(bot, afterWild, 2);
    check(is(bot.chooseAction(wildTop), ActionType::Play, 1, CardColor::None),
          "normal: after a wild the chosen color is what counts");
}

static void testDrawnCard() {
    BotPlayer bot("Bot_1");
    RoundState state = makeState(num(CardColor::Red, 5), CardColor::Red, 0,
                                 true);

    const Card normalDrawn[] = {num(CardColor::Blue, 1), num(CardColor::Red, 2)};
    setHand(bot, normalDrawn, 2);
    check(is(bot.chooseAction(state), ActionType::PlayDrawn, 1, CardColor::None),
          "drawn: a playable drawn card is played immediately");

    const Card wildDrawn[] = {num(CardColor::Blue, 1), num(CardColor::Blue, 2),
                              wild()};
    setHand(bot, wildDrawn, 3);
    check(is(bot.chooseAction(state), ActionType::PlayDrawn, 2, CardColor::Blue),
          "drawn: a drawn wild card gets the best color");

    bot.clearHand();
    check(bot.chooseAction(state).type == ActionType::Pass,
          "drawn: an empty hand falls back to pass");
}

static void testBotBasics() {
    BotPlayer bot("Bot_7");
    check(!bot.isHuman() && bot.getName() == "Bot_7" &&
          bot.getAccount() == nullptr,
          "bot: not human, keeps its name, has no account");
}

int main() {
    cout << "--- Best color ---\n";
    testBestColor();

    cout << "\n--- Penalty mode ---\n";
    testPenaltyMode();

    cout << "\n--- Normal mode ---\n";
    testNormalMode();

    cout << "\n--- Drawn card ---\n";
    testDrawnCard();

    cout << "\n--- Bot basics ---\n";
    testBotBasics();

    cout << "\nFailed checks: " << failedChecks << '\n';
    return failedChecks == 0 ? 0 : 1;
}