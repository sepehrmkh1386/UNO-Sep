#include <iostream>
#include <string>

#include "../src/ds/DynamicArray.h"
#include "../src/model/Account.h"
#include "../src/model/BotPlayer.h"
#include "../src/model/HumanPlayer.h"
#include "../src/logic/TurnManager.h"

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

static string handAsText(const Player& player) {
    string text = "";

    for (int i = 0; i < player.getHandSize(); ++i) {
        Card card;
        player.getCard(i, card);

        if (i > 0) {
            text += "|";
        }

        text += card.toString();
    }

    return text;
}

static void testAccount() {
    Account empty;
    check(empty.getUsername() == "" && empty.getTotalScore() == 0 &&
          empty.getWins() == 0 && empty.getGamesPlayed() == 0,
          "account: default constructor");

    Account account("ali", "1234", "Ali");
    check(account.getUsername() == "ali" && account.getPassword() == "1234" &&
          account.getDisplayName() == "Ali" && account.getTotalScore() == 0,
          "account: constructor fields");

    check(account.checkPassword("1234") && !account.checkPassword("0000") &&
          !account.checkPassword(""),
          "account: checkPassword");

    account.recordGame(120, true);
    check(account.getTotalScore() == 120 && account.getWins() == 1 &&
          account.getGamesPlayed() == 1,
          "account: recordGame with a win");

    account.recordGame(30, false);
    check(account.getTotalScore() == 150 && account.getWins() == 1 &&
          account.getGamesPlayed() == 2,
          "account: recordGame with a loss");

    DynamicArray<Account> accounts;
    accounts.add(Account("sara", "abcd", "Sara"));
    Account loaded;
    check(accounts.get(0, loaded) && loaded.getUsername() == "sara",
          "account: works inside DynamicArray");
}

static void testPlayerBasics() {
    Account account("ali", "1234", "Ali");
    HumanPlayer human("Ali", &account);
    BotPlayer bot("Bot_1");

    check(human.getName() == "Ali" && human.getAccount() == &account &&
          human.isHuman(),
          "player: human has name, account and isHuman");
    check(bot.getName() == "Bot_1" && bot.getAccount() == nullptr &&
          !bot.isHuman(),
          "player: bot has no account and is not human");
}

static void testHand() {
    HumanPlayer player("Ali");
    Card card;

    check(player.getHandSize() == 0 && player.handScore() == 0,
          "hand: starts empty with score 0");

    player.addCard(Card(CardColor::None, CardType::Wild));
    player.addCard(Card(CardColor::Blue, 5));
    player.addCard(Card(CardColor::Red, 9));
    player.addCard(Card(CardColor::Red, CardType::Skip));
    player.addCard(Card(CardColor::Red, 2));
    player.addCard(Card(CardColor::Green, 1));

    check(player.getHandSize() == 6 && player.getCard(0, card) &&
          card.getType() == CardType::Wild,
          "hand: addCard appends to the end");
    check(!player.getCard(6, card) && !player.getCard(-1, card),
          "hand: getCard invalid index fails");

    player.sortHand();
    check(handAsText(player) ==
          "RED 2|RED 9|RED SKIP|GREEN 1|BLUE 5|WILD",
          "hand: sortHand orders by color, then type, then value");

    check(player.handScore() == 87, "hand: handScore adds 2+9+20+1+5+50");

    check(player.removeCard(2, card) && card.getType() == CardType::Skip &&
          player.getHandSize() == 5,
          "hand: removeCard returns the card and shrinks the hand");
    check(!player.removeCard(5, card) && !player.removeCard(-1, card),
          "hand: removeCard invalid index fails");
    check(player.handScore() == 67, "hand: handScore updates after removal");

    player.clearHand();
    check(player.getHandSize() == 0 && player.handScore() == 0,
          "hand: clearHand empties the hand");
}

static void testScores() {
    HumanPlayer player("Ali");

    check(player.getCurrentGameScore() == 0 &&
          player.getRoundScores().isEmpty(),
          "scores: start at zero");

    player.recordRoundScore(100);
    player.recordRoundScore(50);

    int first = 0;
    int second = 0;
    check(player.getCurrentGameScore() == 150 &&
          player.getRoundScores().size() == 2 &&
          player.getRoundScores().get(0, first) && first == 100 &&
          player.getRoundScores().get(1, second) && second == 50,
          "scores: round scores are listed and summed separately");

    player.addCard(Card(CardColor::Red, 1));
    player.clearHand();
    check(player.getCurrentGameScore() == 150,
          "scores: clearHand does not touch the scores");

    player.addCard(Card(CardColor::Red, 1));
    player.resetForNewGame();
    check(player.getCurrentGameScore() == 0 &&
          player.getRoundScores().isEmpty() && player.getHandSize() == 0,
          "scores: resetForNewGame clears hand and scores");
}

static void testActions() {
    HumanPlayer human("Ali");
    RoundState state;

    check(human.chooseAction(state).type == ActionType::None,
          "action: human with no pending action returns None");

    human.setNextAction(Action(ActionType::Play, 2, CardColor::Red));
    Action action = human.chooseAction(state);
    check(action.type == ActionType::Play && action.cardIndex == 2 &&
          action.color == CardColor::Red,
          "action: human returns the action set by the controller");
    check(human.chooseAction(state).type == ActionType::None,
          "action: the pending action is consumed");

    Action draw(ActionType::Draw);
    check(draw.cardIndex == -1 && draw.color == CardColor::None,
          "action: default index and color");
}

static void testTurnManager() {
    TurnManager turns;

    check(turns.isEmpty() && turns.getCurrentPlayer() == nullptr &&
          turns.peekNextPlayer() == nullptr,
          "turns: empty manager returns null players");
    turns.advanceTurn(1);
    check(turns.getCurrentPlayer() == nullptr,
          "turns: advanceTurn on empty manager does nothing");

    HumanPlayer a("A");
    HumanPlayer b("B");
    HumanPlayer c("C");
    turns.addPlayer(&a);
    turns.addPlayer(&b);
    turns.addPlayer(&c);

    check(turns.size() == 3 && turns.getCurrentPlayer() == &a &&
          turns.peekNextPlayer() == &b && turns.isClockwiseDirection(),
          "turns: first added player starts, direction is clockwise");

    turns.advanceTurn(1);
    check(turns.getCurrentPlayer() == &b, "turns: advance 1 goes to B");
    turns.advanceTurn(1);
    check(turns.getCurrentPlayer() == &c, "turns: advance 1 goes to C");
    turns.advanceTurn(1);
    check(turns.getCurrentPlayer() == &a, "turns: advance wraps around to A");

    turns.advanceTurn(2);
    check(turns.getCurrentPlayer() == &c, "turns: advance 2 skips one player");
    turns.advanceTurn(0);
    check(turns.getCurrentPlayer() == &c, "turns: advance 0 does nothing");
    turns.advanceTurn(7);
    check(turns.getCurrentPlayer() == &a,
          "turns: advance 7 with 3 players equals advance 1");

    check(turns.getSeat(0) == &a && turns.getSeat(1) == &b &&
          turns.getSeat(2) == &c && turns.getSeat(3) == nullptr,
          "turns: seats keep the original order");

    turns.reverseDirection();
    check(!turns.isClockwiseDirection() && turns.peekNextPlayer() == &c,
          "turns: reverse makes the previous seat the next player");
    turns.advanceTurn(1);
    check(turns.getCurrentPlayer() == &c, "turns: counter-clockwise goes A to C");
    turns.advanceTurn(1);
    check(turns.getCurrentPlayer() == &b, "turns: counter-clockwise goes C to B");

    turns.reverseDirection();
    check(turns.isClockwiseDirection() && turns.peekNextPlayer() == &c,
          "turns: reversing twice restores the direction");
    check(turns.getSeat(0) == &a, "turns: seats unchanged after reversing");
}

static void testTwoPlayersAndOne() {
    HumanPlayer a("A");
    HumanPlayer b("B");

    TurnManager two;
    two.addPlayer(&a);
    two.addPlayer(&b);

    check(two.peekNextPlayer() == &b, "two players: next is the other player");
    two.advanceTurn(2);
    check(two.getCurrentPlayer() == &a,
          "two players: advance 2 returns to the same player (Skip)");
    two.reverseDirection();
    check(two.peekNextPlayer() == &b,
          "two players: reverse still points at the other player");

    TurnManager one;
    one.addPlayer(&a);
    check(one.peekNextPlayer() == &a, "one player: next is the same player");
    one.advanceTurn(1);
    check(one.getCurrentPlayer() == &a, "one player: advance stays in place");
}

int main() {
    cout << "--- Account tests ---\n";
    testAccount();

    cout << "\n--- Player basics ---\n";
    testPlayerBasics();

    cout << "\n--- Hand tests ---\n";
    testHand();

    cout << "\n--- Score tests ---\n";
    testScores();

    cout << "\n--- Action tests ---\n";
    testActions();

    cout << "\n--- TurnManager tests ---\n";
    testTurnManager();

    cout << "\n--- Two players and one player ---\n";
    testTwoPlayersAndOne();

    cout << "\nFailed checks: " << failedChecks << '\n';
    return failedChecks == 0 ? 0 : 1;
}