#include <iostream>

#include "../src/logic/Round.h"
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

static Card wildDrawFour() {
    return Card(CardColor::None, CardType::WildDrawFour);
}

static void fillSeats(DynamicArray<Player*>& seats,
                      Player* a, Player* b, Player* c) {
    seats.add(a);
    seats.add(b);
    seats.add(c);
}

static void setHand(Player& player, const Card cards[], int count) {
    player.clearHand();

    for (int i = 0; i < count; ++i) {
        player.addCard(cards[i]);
    }
}

static void setOthers(Player& b, Player& c) {
    const Card handB[] = {num(CardColor::Blue, 7), num(CardColor::Blue, 8)};
    const Card handC[] = {num(CardColor::Green, 2), num(CardColor::Green, 3)};
    setHand(b, handB, 2);
    setHand(c, handC, 2);
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

static Round* makePenaltyRound(const DynamicArray<Player*>& seats,
                               Player& a, Player& b, Player& c,
                               bool plusFour) {
    Round* round = makeRound(seats);

    if (round == nullptr) {
        return nullptr;
    }

    if (plusFour) {
        const Card handA[] = {wildDrawFour(), num(CardColor::Green, 1),
                              num(CardColor::Blue, 2)};
        setHand(a, handA, 3);
    } else {
        const Card handA[] = {act(CardColor::Red, CardType::DrawTwo),
                              num(CardColor::Green, 1)};
        setHand(a, handA, 2);
    }

    setOthers(b, c);
    round->setTopCard(num(CardColor::Red, 5));
    round->playCard(0, plusFour ? CardColor::Green : CardColor::None);

    return round;
}

static Round* findDrawnRound(const DynamicArray<Player*>& seats,
                             Player& a, Player& b, Player& c) {
    for (int attempt = 0; attempt < 3000; ++attempt) {
        Round* round = makeRound(seats);

        if (round == nullptr) {
            return nullptr;
        }

        const Card handA[] = {num(CardColor::Green, 1)};
        setHand(a, handA, 1);
        setOthers(b, c);
        round->setTopCard(num(CardColor::Red, 5));

        round->draw();

        if (round->hasDrawnCard()) {
            return round;
        }

        delete round;
    }

    return nullptr;
}

static void testPenaltyDraw() {
    HumanPlayer a("A");
    HumanPlayer b("B");
    HumanPlayer c("C");
    DynamicArray<Player*> seats;
    fillSeats(seats, &a, &b, &c);

    Round* round = makePenaltyRound(seats, a, b, c, false);

    if (round == nullptr) {
        check(false, "penalty: could not set up a round");
        return;
    }

    check(round->getPendingPenalty() == 2 && round->getCurrentPlayer() == &b,
          "penalty: DrawTwo leaves a penalty of 2 for B");
    check(round->playCard(0, CardColor::None) == MoveResult::InvalidMove,
          "penalty: B cannot play a number card");

    int deckBefore = round->getDeckSize();
    check(round->draw() == MoveResult::Ok &&
          round->getLastDrawCount() == 2 && b.getHandSize() == 4 &&
          round->getPendingPenalty() == 0 &&
          round->getCurrentPlayer() == &c &&
          round->getDeckSize() == deckBefore - 2,
          "penalty: draw takes the whole penalty and ends the turn");

    delete round;

    Round* stacked = makeRound(seats);

    if (stacked == nullptr) {
        check(false, "penalty: could not set up the stacked round");
        return;
    }

    const Card handA[] = {act(CardColor::Red, CardType::DrawTwo),
                          num(CardColor::Green, 1)};
    const Card handB[] = {act(CardColor::Blue, CardType::DrawTwo),
                          num(CardColor::Blue, 8)};
    const Card handC[] = {num(CardColor::Green, 2), num(CardColor::Green, 3)};
    setHand(a, handA, 2);
    setHand(b, handB, 2);
    setHand(c, handC, 2);
    stacked->setTopCard(num(CardColor::Red, 5));

    stacked->playCard(0, CardColor::None);
    stacked->playCard(0, CardColor::None);

    check(stacked->getPendingPenalty() == 4 &&
          stacked->getCurrentPlayer() == &c,
          "penalty: two DrawTwo cards stack up to 4 for C");
    check(stacked->draw() == MoveResult::Ok &&
          stacked->getLastDrawCount() == 4 && c.getHandSize() == 6 &&
          stacked->getPendingPenalty() == 0 &&
          stacked->getCurrentPlayer() == &a,
          "penalty: C draws all 4 cards and the turn goes to A");

    delete stacked;
}

static void testNormalDraw() {
    HumanPlayer a("A");
    HumanPlayer b("B");
    HumanPlayer c("C");
    DynamicArray<Player*> seats;
    fillSeats(seats, &a, &b, &c);

    CustomStack<int> noPenalty;
    int playableSeen = 0;
    int unplayableSeen = 0;
    bool consistent = true;

    for (int i = 0; i < 300; ++i) {
        Round* round = makeRound(seats);

        if (round == nullptr) {
            consistent = false;
            break;
        }

        const Card handA[] = {num(CardColor::Green, 1)};
        setHand(a, handA, 1);
        setOthers(b, c);
        round->setTopCard(num(CardColor::Red, 5));

        int deckBefore = round->getDeckSize();
        MoveResult result = round->draw();

        Card top;
        round->getTopCard(top);

        if (result != MoveResult::Ok || round->getLastDrawCount() != 1 ||
            a.getHandSize() != 2 || round->getDeckSize() != deckBefore - 1) {
            consistent = false;
        }

        if (round->hasDrawnCard()) {
            ++playableSeen;

            Card last;
            a.getCard(1, last);

            if (round->getCurrentPlayer() != &a ||
                !RuleManager::isValidPlay(last, top, round->getActiveColor(),
                                          a.getHand(), noPenalty)) {
                consistent = false;
            }
        } else {
            ++unplayableSeen;

            bool anyValid = false;

            for (int j = 0; j < 2; ++j) {
                Card card;
                a.getCard(j, card);

                if (RuleManager::isValidPlay(card, top, round->getActiveColor(),
                                             a.getHand(), noPenalty)) {
                    anyValid = true;
                }
            }

            if (round->getCurrentPlayer() != &b || anyValid) {
                consistent = false;
            }
        }

        delete round;
    }

    check(consistent,
          "draw: one card is drawn, the turn stays only if it is playable");
    check(playableSeen > 0 && unplayableSeen > 0,
          "draw: both outcomes (playable and unplayable) were covered");
}

static void testPlayDrawnAndPass() {
    HumanPlayer a("A");
    HumanPlayer b("B");
    HumanPlayer c("C");
    DynamicArray<Player*> seats;
    fillSeats(seats, &a, &b, &c);

    Round* playRound = findDrawnRound(seats, a, b, c);

    if (playRound == nullptr) {
        check(false, "play_drawn: could not find a playable draw");
        return;
    }

    check(playRound->playCard(0, CardColor::None) == MoveResult::NotAllowed &&
          playRound->draw() == MoveResult::NotAllowed,
          "play_drawn: play and draw are blocked while a decision is pending");
    check(playRound->playDrawn(CardColor::Red) == MoveResult::Ok &&
          a.getHandSize() == 1 && !playRound->hasDrawnCard(),
          "play_drawn: the drawn card is played");

    Card top;
    check(playRound->getTopCard(top) &&
          top.getColor() == playRound->getActiveColor(),
          "play_drawn: the played card is on the table");
    check(playRound->playDrawn(CardColor::Red) == MoveResult::NotAllowed,
          "play_drawn: not allowed again without a new draw");
    delete playRound;

    Round* passRound = findDrawnRound(seats, a, b, c);

    if (passRound == nullptr) {
        check(false, "pass: could not find a playable draw");
        return;
    }

    check(passRound->pass() == MoveResult::Ok && a.getHandSize() == 2 &&
          passRound->getCurrentPlayer() == &b && !passRound->hasDrawnCard(),
          "pass: keeps the drawn card and moves the turn to B");
    check(passRound->pass() == MoveResult::NotAllowed,
          "pass: not allowed when nothing was drawn");
    delete passRound;

    Round* fresh = makeRound(seats);

    if (fresh == nullptr) {
        check(false, "pass: could not set up a fresh round");
        return;
    }

    check(fresh->pass() == MoveResult::NotAllowed &&
          fresh->playDrawn(CardColor::Red) == MoveResult::NotAllowed,
          "pass and play_drawn: not allowed before drawing");
    delete fresh;
}

static void testChance() {
    HumanPlayer a("A");
    HumanPlayer b("B");
    HumanPlayer c("C");
    DynamicArray<Player*> seats;
    fillSeats(seats, &a, &b, &c);

    Round* none = makeRound(seats);

    if (none == nullptr) {
        check(false, "chance: could not set up a round");
        return;
    }

    check(none->chance() == MoveResult::NotAllowed,
          "chance: not allowed without a pending penalty");
    delete none;

    Round* zero = makePenaltyRound(seats, a, b, c, false);
    check(zero != nullptr && zero->chanceWithRoll(6) == MoveResult::InvalidMove &&
          zero->getPendingPenalty() == 2 && zero->getCurrentPlayer() == &b,
          "chance: an invalid roll changes nothing");
    check(zero->chanceWithRoll(0) == MoveResult::Ok &&
          zero->getLastDrawCount() == 0 && b.getHandSize() == 2 &&
          zero->getPendingPenalty() == 0 && zero->getCurrentPlayer() == &c,
          "chance: +2 with roll 0 means no cards, the penalty is cleared");
    delete zero;

    Round* four = makePenaltyRound(seats, a, b, c, false);
    check(four != nullptr && four->chanceWithRoll(5) == MoveResult::Ok &&
          four->getLastDrawCount() == 4 && b.getHandSize() == 6 &&
          four->getCurrentPlayer() == &c,
          "chance: +2 with roll 5 means 4 cards");
    delete four;

    Round* plusFour = makePenaltyRound(seats, a, b, c, true);
    check(plusFour != nullptr && plusFour->getPendingPenalty() == 4 &&
          plusFour->chanceWithRoll(1) == MoveResult::Ok &&
          plusFour->getLastDrawCount() == 2 && b.getHandSize() == 4,
          "chance: +4 with roll 1 means 2 cards");
    delete plusFour;

    Round* six = makePenaltyRound(seats, a, b, c, true);
    check(six != nullptr && six->chanceWithRoll(5) == MoveResult::Ok &&
          six->getLastDrawCount() == 6 && b.getHandSize() == 8,
          "chance: +4 with roll 5 means 6 cards");
    delete six;

    bool randomOk = true;

    for (int i = 0; i < 200; ++i) {
        Round* round = makePenaltyRound(seats, a, b, c, false);

        if (round == nullptr || round->chance() != MoveResult::Ok) {
            randomOk = false;
            delete round;
            break;
        }

        int count = round->getLastDrawCount();

        if (!(count == 0 || count == 1 || count == 2 || count == 4) ||
            b.getHandSize() != 2 + count ||
            round->getPendingPenalty() != 0) {
            randomOk = false;
        }

        delete round;
    }

    check(randomOk, "chance: random spins give 0, 1, 2 or 4 cards for +2");
}

static void testApplyActionAndState() {
    HumanPlayer a("A");
    HumanPlayer b("B");
    HumanPlayer c("C");
    DynamicArray<Player*> seats;
    fillSeats(seats, &a, &b, &c);

    Round* round = makeRound(seats);

    if (round == nullptr) {
        check(false, "apply: could not set up a round");
        return;
    }

    const Card handA[] = {num(CardColor::Red, 9), num(CardColor::Green, 1)};
    setHand(a, handA, 2);
    setOthers(b, c);
    round->setTopCard(num(CardColor::Red, 5));

    check(round->applyAction(Action()) == MoveResult::NotAllowed,
          "apply: None is not allowed");
    check(round->applyAction(Action(ActionType::Play, 0)) == MoveResult::Ok &&
          round->getCurrentPlayer() == &b,
          "apply: Play works through applyAction");
    check(round->applyAction(Action(ActionType::Pass)) == MoveResult::NotAllowed,
          "apply: Pass is not allowed before drawing");
    check(round->applyAction(Action(ActionType::Draw)) == MoveResult::Ok &&
          round->getLastDrawCount() == 1 && b.getHandSize() == 3,
          "apply: Draw works through applyAction");
    delete round;

    Round* penalty = makePenaltyRound(seats, a, b, c, false);

    if (penalty == nullptr) {
        check(false, "state: could not set up a penalty round");
        return;
    }

    RoundState state = penalty->getState();
    check(state.pendingPenalty == 2 && state.activeColor == CardColor::Red &&
          state.topCard.getType() == CardType::DrawTwo && !state.hasDrawn,
          "state: getState describes the table");
    delete penalty;

    Round* drawn = findDrawnRound(seats, a, b, c);
    check(drawn != nullptr && drawn->getState().hasDrawn,
          "state: hasDrawn is true while a drawn card is pending");
    delete drawn;
}

static void testLeaveOneCard() {
    HumanPlayer a("A");
    HumanPlayer b("B");
    HumanPlayer c("C");
    DynamicArray<Player*> seats;
    fillSeats(seats, &a, &b, &c);

    Round* round = makeRound(seats);

    if (round == nullptr) {
        check(false, "leave one: could not set up a round");
        return;
    }

    round->leaveOneCardInDeck();
    check(round->getDeckSize() == 1, "leave one: only 1 card stays in the deck");

    const Card handA[] = {num(CardColor::Green, 1)};
    setHand(a, handA, 1);
    round->setTopCard(num(CardColor::Red, 5));

    check(round->draw() == MoveResult::Ok && round->getLastDrawCount() == 1 &&
          round->getDeckSize() == 0,
          "leave one: the last card can still be drawn");
    delete round;
}

int main() {
    cout << "--- Penalty draw ---\n";
    testPenaltyDraw();

    cout << "\n--- Normal draw ---\n";
    testNormalDraw();

    cout << "\n--- play_drawn and pass ---\n";
    testPlayDrawnAndPass();

    cout << "\n--- Chance machine ---\n";
    testChance();

    cout << "\n--- applyAction and state ---\n";
    testApplyActionAndState();

    cout << "\n--- Deck with one card ---\n";
    testLeaveOneCard();

    cout << "\nFailed checks: " << failedChecks << '\n';
    return failedChecks == 0 ? 0 : 1;
}