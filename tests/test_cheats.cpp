#include <iostream>

#include "../src/logic/Game.h"
#include "../src/model/HumanPlayer.h"

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

static void setHand(Player& player, const Card cards[], int count) {
    player.clearHand();

    for (int i = 0; i < count; ++i) {
        player.addCard(cards[i]);
    }
}

static int totalCards(Round& round) {
    int total = round.getDeckSize() + round.getDiscardSize();

    for (int i = 0; i < round.getPlayerCount(); ++i) {
        total += round.getPlayerAt(i)->getHandSize();
    }

    return total;
}

static void fillSeats(DynamicArray<Player*>& seats,
                      Player* a, Player* b, Player* c) {
    seats.add(a);
    seats.add(b);
    seats.add(c);
}

static Round* makeRound(const DynamicArray<Player*>& seats) {
    for (int attempt = 0; attempt < 20000; ++attempt) {
        Round* round = new Round(seats);
        round->start();

        Card top;

        if (round->getTopCard(top) && top.isNumberCard()) {
            return round;
        }

        delete round;
    }

    return nullptr;
}

static void setOthers(Player& b, Player& c) {
    const Card handB[] = {num(CardColor::Blue, 7), num(CardColor::Blue, 8)};
    const Card handC[] = {num(CardColor::Green, 2), num(CardColor::Green, 3)};
    setHand(b, handB, 2);
    setHand(c, handC, 2);
}

static void testCheatGive() {
    HumanPlayer a("A");
    HumanPlayer b("B");
    HumanPlayer c("C");
    DynamicArray<Player*> seats;
    fillSeats(seats, &a, &b, &c);

    Round* round = makeRound(seats);

    if (round == nullptr) {
        check(false, "give: could not set up a round");
        return;
    }

    const Card handA[] = {num(CardColor::Red, 1)};
    setHand(a, handA, 1);
    setOthers(b, c);
    round->setTopCard(num(CardColor::Red, 5));

    Card wild(CardColor::None, CardType::WildDrawFour);
    Card last;

    check(round->cheatGive(wild) && a.getHandSize() == 2 &&
          a.getCard(1, last) && last.getType() == CardType::WildDrawFour,
          "give: the card is added to the end of the current hand");
    check(round->getCurrentPlayer() == &a,
          "give: the turn does not change");

    check(round->cheatWinRound() && !round->cheatGive(wild),
          "give: nothing can be given after the round is over");
    delete round;
}

static void testCheatSetTop() {
    HumanPlayer a("A");
    HumanPlayer b("B");
    HumanPlayer c("C");
    DynamicArray<Player*> seats;
    fillSeats(seats, &a, &b, &c);

    Round* round = makeRound(seats);

    if (round == nullptr) {
        check(false, "top: could not set up a round");
        return;
    }

    const Card handA[] = {act(CardColor::Red, CardType::DrawTwo),
                          num(CardColor::Green, 1)};
    setHand(a, handA, 2);
    setOthers(b, c);
    round->setTopCard(num(CardColor::Red, 5));
    round->playCard(0, CardColor::None);

    int discardBefore = round->getDiscardSize();
    check(round->getPendingPenalty() == 2 && round->getCurrentPlayer() == &b,
          "top: setup has a pending penalty for B");

    round->cheatSetTop(num(CardColor::Blue, 3));

    Card top;
    check(round->getTopCard(top) && top.getColor() == CardColor::Blue &&
          top.getValue() == 3 && round->getActiveColor() == CardColor::Blue,
          "top: the top card and the active color change");
    check(round->getDiscardSize() == discardBefore,
          "top: the old top card is replaced, not stacked");
    check(round->getPendingPenalty() == 2 && round->getCurrentPlayer() == &b,
          "top: the penalty and the turn are untouched");

    Card wild(CardColor::None, CardType::Wild);
    wild.setColor(CardColor::Green);
    round->cheatSetTop(wild);
    check(round->getTopCard(top) && top.getType() == CardType::Wild &&
          round->getActiveColor() == CardColor::Green,
          "top: a wild card with a color sets the active color");
    delete round;
}

static void testEmptyDeck() {
    HumanPlayer a("A");
    HumanPlayer b("B");
    HumanPlayer c("C");
    DynamicArray<Player*> seats;
    fillSeats(seats, &a, &b, &c);

    Round* round = makeRound(seats);

    if (round == nullptr) {
        check(false, "empty deck: could not set up a round");
        return;
    }

    const Card handA[] = {act(CardColor::Red, CardType::DrawTwo),
                          num(CardColor::Green, 1)};
    setHand(a, handA, 2);
    setOthers(b, c);
    round->setTopCard(num(CardColor::Red, 5));

    int totalBefore = totalCards(*round);
    Card topBefore;
    round->getTopCard(topBefore);

    round->leaveOneCardInDeck();

    Card topAfter;
    check(round->getDeckSize() == 1, "empty deck: only 1 card stays in the deck");
    check(round->getTopCard(topAfter) &&
          topAfter.toString() == topBefore.toString(),
          "empty deck: the top card is untouched");
    check(totalCards(*round) == totalBefore,
          "empty deck: no card is lost");

    round->playCard(0, CardColor::None);
    int bBefore = b.getHandSize();
    int totalMid = totalCards(*round);

    check(round->draw() == MoveResult::Ok && b.getHandSize() == bBefore + 2,
          "empty deck: a penalty draw works across the reshuffle");
    check(totalCards(*round) == totalMid,
          "empty deck: no card is lost or duplicated by the reshuffle");

    Card topNow;
    check(round->getTopCard(topNow) && topNow.getType() == CardType::DrawTwo,
          "empty deck: the table card stays on the table");
    check(round->getDeckSize() > 50,
          "empty deck: the deck was rebuilt from the discard pile");
    delete round;
}

static void testWinRound() {
    HumanPlayer a("A");
    HumanPlayer b("B");
    HumanPlayer c("C");
    DynamicArray<Player*> seats;
    fillSeats(seats, &a, &b, &c);

    Round* round = makeRound(seats);

    if (round == nullptr) {
        check(false, "win: could not set up a round");
        return;
    }

    Player* current = round->getCurrentPlayer();
    check(!round->isOver() && current->getHandSize() == 7,
          "win: the round is running with 7 cards in hand");
    check(round->cheatWinRound(), "win: the cheat is accepted");
    check(round->isOver() && round->getWinner() == current &&
          current->getHandSize() == 1,
          "win: the round ends, the current player wins with 1 card left");
    check(round->playCard(0, CardColor::None) == MoveResult::RoundOver &&
          round->draw() == MoveResult::RoundOver,
          "win: no moves are accepted afterwards");
    check(!round->cheatWinRound(), "win: it cannot be used twice");
    delete round;

    Game game(seats, GameMode::Score, 100000);
    game.start();
    Round* gameRound = game.getRound();
    Player* gameWinner = gameRound->getCurrentPlayer();

    gameRound->cheatWinRound();

    int expected = 0;

    for (int i = 0; i < game.getPlayerCount(); ++i) {
        if (game.getPlayerAt(i) != gameWinner) {
            expected += game.getPlayerAt(i)->handScore();
        }
    }

    check(game.finishRound() && game.getRoundsPlayed() == 1 &&
          game.getLastRoundPoints() == expected &&
          gameWinner->getCurrentGameScore() == expected,
          "win: the score is the sum of the other hands (recursive scoring)");
}

static void testLastDrawnCard() {
    HumanPlayer a("A");
    HumanPlayer b("B");
    HumanPlayer c("C");
    DynamicArray<Player*> seats;
    fillSeats(seats, &a, &b, &c);

    bool matched = false;

    for (int attempt = 0; attempt < 3000 && !matched; ++attempt) {
        Round* round = makeRound(seats);

        if (round == nullptr) {
            break;
        }

        const Card handA[] = {num(CardColor::Green, 1)};
        setHand(a, handA, 1);
        setOthers(b, c);
        round->setTopCard(num(CardColor::Red, 5));

        Card none;
        check(attempt > 0 || !round->getLastDrawnCard(none),
              "drawn card: nothing is reported before a draw");

        round->draw();

        Card drawn;
        Card last;

        if (round->hasDrawnCard() && round->getLastDrawnCard(drawn) &&
            a.getCard(a.getHandSize() - 1, last) &&
            drawn.toString() == last.toString() &&
            round->getLastDrawCount() == 1) {
            matched = true;
        }

        delete round;
    }

    check(matched, "drawn card: it is the last card of the hand after a draw");
}

int main() {
    cout << "--- cheat_give ---\n";
    testCheatGive();

    cout << "\n--- cheat_top ---\n";
    testCheatSetTop();

    cout << "\n--- cheat_empty_deck ---\n";
    testEmptyDeck();

    cout << "\n--- cheat_win_round ---\n";
    testWinRound();

    cout << "\n--- last drawn card ---\n";
    testLastDrawnCard();

    cout << "\nFailed checks: " << failedChecks << '\n';
    return failedChecks == 0 ? 0 : 1;
}