#include <iostream>
#include <string>

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

static Card wild() {
    return Card(CardColor::None, CardType::Wild);
}

static Card wildDrawFour() {
    return Card(CardColor::None, CardType::WildDrawFour);
}

static void fillSeats(DynamicArray<Player*>& seats,
                      Player* a, Player* b, Player* c) {
    seats.add(a);
    seats.add(b);

    if (c != nullptr) {
        seats.add(c);
    }
}

static void setHand(Player& player, const Card cards[], int count) {
    player.clearHand();

    for (int i = 0; i < count; ++i) {
        player.addCard(cards[i]);
    }
}

static bool isNumberStart(const Card& card) {
    return card.isNumberCard();
}

static bool isWildStart(const Card& card) {
    return card.getType() == CardType::Wild;
}

static Round* makeRoundWhere(const DynamicArray<Player*>& seats,
                             bool (*accept)(const Card&)) {
    for (int attempt = 0; attempt < 20000; ++attempt) {
        Round* round = new Round(seats);
        round->start();

        Card top;

        if (round->getTopCard(top) && accept(top)) {
            return round;
        }

        delete round;
    }

    return nullptr;
}

static void testDeckReturn() {
    Deck deck;
    DiscardPile discard;
    deck.build();

    Card card;
    check(deck.drawCard(card, discard) && deck.size() == 107,
          "deck: one card drawn leaves 107");

    deck.returnAndReshuffle(card);
    check(deck.size() == 108, "deck: returnAndReshuffle puts the card back");

    int typeCount[6] = {0, 0, 0, 0, 0, 0};
    bool allDrawn = true;

    for (int i = 0; i < 108; ++i) {
        Card drawn;

        if (!deck.drawCard(drawn, discard)) {
            allDrawn = false;
        } else {
            ++typeCount[static_cast<int>(drawn.getType())];
        }
    }

    check(allDrawn && deck.isEmpty() && discard.isEmpty(),
          "deck: all 108 cards can still be drawn after reshuffling");
    check(typeCount[0] == 76 && typeCount[1] == 8 && typeCount[2] == 8 &&
          typeCount[3] == 8 && typeCount[4] == 4 && typeCount[5] == 4,
          "deck: no card was lost or duplicated by returnAndReshuffle");

    Deck empty;
    empty.returnAndReshuffle(Card(CardColor::Red, 1));
    check(empty.size() == 1, "deck: returnAndReshuffle works on an empty deck");
}

static void testStartInvariants() {
    HumanPlayer a("A");
    HumanPlayer b("B");
    HumanPlayer c("C");
    DynamicArray<Player*> seats;
    fillSeats(seats, &a, &b, &c);

    bool conserved = true;
    bool noWildDrawFour = true;
    bool effectsOk = true;
    int seen[6] = {0, 0, 0, 0, 0, 0};

    for (int i = 0; i < 400; ++i) {
        Round round(seats);
        round.start();

        int total = round.getDeckSize() + round.getDiscardSize();
        for (int s = 0; s < 3; ++s) {
            total += round.getPlayerAt(s)->getHandSize();
        }

        if (total != 108) {
            conserved = false;
        }

        Card top;
        round.getTopCard(top);
        ++seen[static_cast<int>(top.getType())];

        if (top.getType() == CardType::WildDrawFour) {
            noWildDrawFour = false;
        }

        Player* current = round.getCurrentPlayer();
        bool sevenEach = a.getHandSize() == 7 && b.getHandSize() == 7 &&
                         c.getHandSize() == 7;

        switch (top.getType()) {
            case CardType::Number:
                effectsOk = effectsOk && current == &a && sevenEach &&
                            round.isClockwise() &&
                            round.getActiveColor() == top.getColor();
                break;
            case CardType::Skip:
                effectsOk = effectsOk && current == &b && sevenEach &&
                            round.isClockwise() &&
                            round.getActiveColor() == top.getColor();
                break;
            case CardType::Reverse:
                effectsOk = effectsOk && current == &a && sevenEach &&
                            !round.isClockwise() &&
                            round.getActiveColor() == top.getColor();
                break;
            case CardType::DrawTwo:
                effectsOk = effectsOk && current == &b &&
                            a.getHandSize() == 9 && b.getHandSize() == 7 &&
                            c.getHandSize() == 7 &&
                            round.getActiveColor() == top.getColor();
                break;
            case CardType::Wild:
                effectsOk = effectsOk && current == &a && sevenEach &&
                            round.needsStartColor() &&
                            round.getActiveColor() == CardColor::None;
                break;
            case CardType::WildDrawFour:
                effectsOk = false;
                break;
        }
    }

    check(conserved, "start: 108 cards are always accounted for");
    check(noWildDrawFour, "start: WildDrawFour is never the first card");
    check(effectsOk, "start: first card effects are applied correctly");
    check(seen[0] > 0 && seen[1] > 0 && seen[2] > 0 &&
          seen[3] > 0 && seen[4] > 0,
          "start: every kind of first card was covered by the sample");
}

static void testStartColor() {
    HumanPlayer a("A");
    HumanPlayer b("B");
    HumanPlayer c("C");
    DynamicArray<Player*> seats;
    fillSeats(seats, &a, &b, &c);

    Round* round = makeRoundWhere(seats, isWildStart);

    if (round == nullptr) {
        check(false, "start color: could not find a round starting with Wild");
        return;
    }

    check(round->needsStartColor() && round->getCurrentPlayer() == &a,
          "start color: first player must choose the color");
    check(round->playCard(0, CardColor::Red) == MoveResult::AwaitingStartColor,
          "start color: playing is blocked until the color is chosen");
    check(!round->setStartColor(CardColor::None),
          "start color: None is not a valid choice");
    check(round->setStartColor(CardColor::Blue) &&
          round->getActiveColor() == CardColor::Blue &&
          !round->needsStartColor(),
          "start color: the chosen color becomes the active color");

    Card top;
    check(round->getTopCard(top) && top.getType() == CardType::Wild &&
          top.getColor() == CardColor::Blue,
          "start color: the wild card on the table shows the chosen color");
    check(!round->setStartColor(CardColor::Red) &&
          round->getActiveColor() == CardColor::Blue,
          "start color: the color cannot be changed afterwards");

    delete round;
}

static void testPlayBasics() {
    HumanPlayer a("A");
    HumanPlayer b("B");
    HumanPlayer c("C");
    DynamicArray<Player*> seats;
    fillSeats(seats, &a, &b, &c);

    Round* round = makeRoundWhere(seats, isNumberStart);

    if (round == nullptr) {
        check(false, "play: could not set up a round");
        return;
    }

    const Card handA[] = {num(CardColor::Red, 9), num(CardColor::Blue, 3),
                          num(CardColor::Green, 7)};
    const Card handB[] = {num(CardColor::Red, 1), num(CardColor::Red, 2)};
    const Card handC[] = {num(CardColor::Red, 3), num(CardColor::Red, 4)};
    setHand(a, handA, 3);
    setHand(b, handB, 2);
    setHand(c, handC, 2);
    round->setTopCard(num(CardColor::Red, 5));

    check(round->getCurrentPlayer() == &a && round->isClockwise(),
          "play: A starts and the direction is clockwise");

    check(round->playCard(1, CardColor::None) == MoveResult::InvalidMove &&
          a.getHandSize() == 3 && round->getCurrentPlayer() == &a,
          "play: an invalid card is rejected and the turn is kept");
    check(round->playCard(9, CardColor::None) == MoveResult::InvalidIndex &&
          round->playCard(-1, CardColor::None) == MoveResult::InvalidIndex,
          "play: an invalid index is rejected");

    check(round->playCard(0, CardColor::None) == MoveResult::Ok,
          "play: a card of the same color is accepted");

    Card top;
    check(round->getTopCard(top) && top.getValue() == 9 &&
          round->getActiveColor() == CardColor::Red,
          "play: the played card is on top of the discard pile");
    check(a.getHandSize() == 2 && round->getCurrentPlayer() == &b,
          "play: the card left the hand and the turn moved to B");

    Card first;
    check(a.getCard(0, first) && first.toString() == "GREEN 7",
          "play: the hand is sorted after playing");

    check(round->playCard(0, CardColor::None) == MoveResult::Ok &&
          round->getCurrentPlayer() == &c,
          "play: B plays and the turn moves to C");

    Card blueFive = num(CardColor::Blue, 5);
    round->setTopCard(blueFive);
    const Card newHandC[] = {num(CardColor::Green, 5), num(CardColor::Red, 8)};
    setHand(c, newHandC, 2);
    check(round->playCard(0, CardColor::None) == MoveResult::Ok &&
          round->getActiveColor() == CardColor::Green,
          "play: a card with the same number is accepted and sets the color");

    delete round;
}

static void testSkipAndReverse() {
    HumanPlayer a("A");
    HumanPlayer b("B");
    HumanPlayer c("C");
    DynamicArray<Player*> seats;
    fillSeats(seats, &a, &b, &c);

    Round* skipRound = makeRoundWhere(seats, isNumberStart);

    if (skipRound == nullptr) {
        check(false, "skip: could not set up a round");
        return;
    }

    const Card skipHand[] = {act(CardColor::Red, CardType::Skip),
                             num(CardColor::Green, 1)};
    const Card others[] = {num(CardColor::Red, 1), num(CardColor::Red, 2)};
    setHand(a, skipHand, 2);
    setHand(b, others, 2);
    setHand(c, others, 2);
    skipRound->setTopCard(num(CardColor::Red, 5));

    check(skipRound->playCard(0, CardColor::None) == MoveResult::Ok &&
          skipRound->getCurrentPlayer() == &c,
          "skip: the next player loses the turn (A to C)");
    delete skipRound;

    Round* reverseRound = makeRoundWhere(seats, isNumberStart);

    if (reverseRound == nullptr) {
        check(false, "reverse: could not set up a round");
        return;
    }

    const Card reverseHand[] = {act(CardColor::Red, CardType::Reverse),
                                num(CardColor::Green, 1)};
    setHand(a, reverseHand, 2);
    setHand(b, others, 2);
    setHand(c, others, 2);
    reverseRound->setTopCard(num(CardColor::Red, 5));

    check(reverseRound->playCard(0, CardColor::None) == MoveResult::Ok &&
          !reverseRound->isClockwise() &&
          reverseRound->getCurrentPlayer() == &c,
          "reverse: 3 players, direction flips and the previous seat plays");
    check(reverseRound->playCard(0, CardColor::None) == MoveResult::Ok &&
          reverseRound->getCurrentPlayer() == &b,
          "reverse: the turn keeps moving in the new direction (C to B)");
    delete reverseRound;

    HumanPlayer d("D");
    HumanPlayer e("E");
    DynamicArray<Player*> twoSeats;
    fillSeats(twoSeats, &d, &e, nullptr);

    Round* twoRound = makeRoundWhere(twoSeats, isNumberStart);

    if (twoRound == nullptr) {
        check(false, "reverse 2p: could not set up a round");
        return;
    }

    const Card twoHand[] = {act(CardColor::Blue, CardType::Reverse),
                            num(CardColor::Green, 1)};
    const Card twoOther[] = {num(CardColor::Blue, 2), num(CardColor::Blue, 3)};
    setHand(d, twoHand, 2);
    setHand(e, twoOther, 2);
    twoRound->setTopCard(num(CardColor::Blue, 5));

    check(twoRound->playCard(0, CardColor::None) == MoveResult::Ok &&
          twoRound->getCurrentPlayer() == &d,
          "reverse 2p: acts like Skip, the same player plays again");
    delete twoRound;
}

static void testWildCards() {
    HumanPlayer a("A");
    HumanPlayer b("B");
    HumanPlayer c("C");
    DynamicArray<Player*> seats;
    fillSeats(seats, &a, &b, &c);

    Round* round = makeRoundWhere(seats, isNumberStart);

    if (round == nullptr) {
        check(false, "wild: could not set up a round");
        return;
    }

    const Card handA[] = {wild(), num(CardColor::Red, 1)};
    const Card handB[] = {num(CardColor::Blue, 2), num(CardColor::Red, 3)};
    const Card handC[] = {num(CardColor::Red, 4), num(CardColor::Red, 6)};
    setHand(a, handA, 2);
    setHand(b, handB, 2);
    setHand(c, handC, 2);
    round->setTopCard(num(CardColor::Red, 5));

    check(round->playCard(0, CardColor::None) == MoveResult::ColorRequired &&
          a.getHandSize() == 2 && round->getCurrentPlayer() == &a,
          "wild: playing without a color is rejected and nothing changes");
    check(round->playCard(0, CardColor::Blue) == MoveResult::Ok &&
          round->getActiveColor() == CardColor::Blue &&
          round->getCurrentPlayer() == &b,
          "wild: the chosen color becomes the active color");

    Card top;
    check(round->getTopCard(top) && top.getType() == CardType::Wild &&
          top.getColor() == CardColor::Blue,
          "wild: the card on the table carries the chosen color");

    check(round->playCard(1, CardColor::None) == MoveResult::InvalidMove,
          "wild: a card of another color is rejected afterwards");
    check(round->playCard(0, CardColor::None) == MoveResult::Ok,
          "wild: a card of the chosen color is accepted");

    delete round;
}

static void testWildDrawFour() {
    HumanPlayer a("A");
    HumanPlayer b("B");
    HumanPlayer c("C");
    DynamicArray<Player*> seats;
    fillSeats(seats, &a, &b, &c);

    const Card others[] = {num(CardColor::Red, 1), num(CardColor::Red, 2)};

    Round* blocked = makeRoundWhere(seats, isNumberStart);

    if (blocked == nullptr) {
        check(false, "wd4: could not set up a round");
        return;
    }

    const Card withRed[] = {wildDrawFour(), num(CardColor::Red, 9),
                            num(CardColor::Green, 1)};
    setHand(a, withRed, 3);
    setHand(b, others, 2);
    setHand(c, others, 2);
    blocked->setTopCard(num(CardColor::Red, 5));

    check(blocked->playCard(0, CardColor::Blue) == MoveResult::InvalidMove &&
          a.getHandSize() == 3 && blocked->getPendingPenalty() == 0,
          "wd4: rejected while the hand has a card of the active color");
    delete blocked;

    Round* allowed = makeRoundWhere(seats, isNumberStart);

    if (allowed == nullptr) {
        check(false, "wd4: could not set up the second round");
        return;
    }

    const Card noRed[] = {wildDrawFour(), num(CardColor::Green, 1),
                          num(CardColor::Blue, 2)};
    setHand(a, noRed, 3);
    setHand(b, others, 2);
    setHand(c, others, 2);
    allowed->setTopCard(num(CardColor::Red, 5));

    check(allowed->playCard(0, CardColor::Green) == MoveResult::Ok &&
          allowed->getPendingPenalty() == 4 &&
          allowed->getActiveColor() == CardColor::Green &&
          allowed->getCurrentPlayer() == &b,
          "wd4: accepted without an active-color card, adds 4 to the penalty");
    delete allowed;
}

static void testStacking() {
    HumanPlayer a("A");
    HumanPlayer b("B");
    HumanPlayer c("C");
    DynamicArray<Player*> seats;
    fillSeats(seats, &a, &b, &c);

    Round* round = makeRoundWhere(seats, isNumberStart);

    if (round == nullptr) {
        check(false, "stacking: could not set up a round");
        return;
    }

    const Card handA[] = {act(CardColor::Red, CardType::DrawTwo),
                          num(CardColor::Green, 1)};
    const Card handB[] = {act(CardColor::Blue, CardType::DrawTwo),
                          num(CardColor::Red, 7)};
    const Card handC[] = {wildDrawFour(), num(CardColor::Blue, 3),
                          num(CardColor::Green, 2)};
    setHand(a, handA, 2);
    setHand(b, handB, 2);
    setHand(c, handC, 3);
    round->setTopCard(num(CardColor::Red, 5));

    check(round->playCard(0, CardColor::None) == MoveResult::Ok &&
          round->getPendingPenalty() == 2 && round->getCurrentPlayer() == &b,
          "stacking: DrawTwo adds 2 and passes the turn");

    check(round->playCard(1, CardColor::None) == MoveResult::InvalidMove &&
          b.getHandSize() == 2,
          "stacking: a number card is rejected while a penalty is pending");

    check(round->playCard(0, CardColor::None) == MoveResult::Ok &&
          round->getPendingPenalty() == 4 &&
          round->getActiveColor() == CardColor::Blue &&
          round->getCurrentPlayer() == &c,
          "stacking: a second DrawTwo of another color raises it to 4");

    check(round->playCard(0, CardColor::Yellow) == MoveResult::Ok &&
          round->getPendingPenalty() == 8 &&
          round->getActiveColor() == CardColor::Yellow &&
          round->getCurrentPlayer() == &a,
          "stacking: WildDrawFour ignores the color restriction, penalty is 8");

    delete round;
}

static void testWin() {
    HumanPlayer a("A");
    HumanPlayer b("B");
    HumanPlayer c("C");
    DynamicArray<Player*> seats;
    fillSeats(seats, &a, &b, &c);

    Round* round = makeRoundWhere(seats, isNumberStart);

    if (round == nullptr) {
        check(false, "win: could not set up a round");
        return;
    }

    const Card last[] = {num(CardColor::Red, 9)};
    const Card others[] = {num(CardColor::Red, 1), num(CardColor::Red, 2)};
    setHand(a, last, 1);
    setHand(b, others, 2);
    setHand(c, others, 2);
    round->setTopCard(num(CardColor::Red, 5));

    check(!round->isOver() && round->getWinner() == nullptr,
          "win: the round is running at first");
    check(round->playCard(0, CardColor::None) == MoveResult::Ok &&
          round->isOver() && round->getWinner() == &a &&
          a.getHandSize() == 0,
          "win: playing the last card ends the round with A as the winner");
    check(round->playCard(0, CardColor::None) == MoveResult::RoundOver,
          "win: no more moves are accepted after the round is over");

    delete round;
}

int main() {
    cout << "--- Deck returnAndReshuffle ---\n";
    testDeckReturn();

    cout << "\n--- Start invariants ---\n";
    testStartInvariants();

    cout << "\n--- Start color (first card Wild) ---\n";
    testStartColor();

    cout << "\n--- Play basics ---\n";
    testPlayBasics();

    cout << "\n--- Skip and Reverse ---\n";
    testSkipAndReverse();

    cout << "\n--- Wild cards ---\n";
    testWildCards();

    cout << "\n--- WildDrawFour ---\n";
    testWildDrawFour();

    cout << "\n--- Stacking ---\n";
    testStacking();

    cout << "\n--- Winning ---\n";
    testWin();

    cout << "\nFailed checks: " << failedChecks << '\n';
    return failedChecks == 0 ? 0 : 1;
}