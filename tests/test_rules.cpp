#include <iostream>

#include "../src/logic/RuleManager.h"

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

static bool valid(const Card& card, const Card& top, CardColor active,
                  const SinglyLinkedList<Card>& hand,
                  const CustomStack<int>& penalty) {
    return RuleManager::isValidPlay(card, top, active, hand, penalty);
}

static void testColorAndNumber() {
    SinglyLinkedList<Card> hand;
    CustomStack<int> penalty;
    Card top = num(CardColor::Red, 5);
    CardColor active = CardColor::Red;

    check(valid(num(CardColor::Red, 9), top, active, hand, penalty),
          "match: same color, different number");
    check(valid(num(CardColor::Blue, 5), top, active, hand, penalty),
          "match: same number, different color");
    check(!valid(num(CardColor::Blue, 9), top, active, hand, penalty),
          "match: different color and number is invalid");
    check(valid(act(CardColor::Red, CardType::Skip), top, active, hand, penalty),
          "match: action card with the active color");
    check(!valid(act(CardColor::Blue, CardType::Skip), top, active, hand, penalty),
          "match: action card with another color on a number is invalid");

    Card zeroTop = num(CardColor::Green, 0);
    check(valid(num(CardColor::Yellow, 0), zeroTop, CardColor::Green, hand, penalty),
          "match: zero matches zero");
    check(!valid(num(CardColor::Yellow, 1), zeroTop, CardColor::Green, hand, penalty),
          "match: one does not match zero");
}

static void testActionTypes() {
    SinglyLinkedList<Card> hand;
    CustomStack<int> penalty;

    Card skipTop = act(CardColor::Red, CardType::Skip);
    check(valid(act(CardColor::Blue, CardType::Skip), skipTop,
                CardColor::Red, hand, penalty),
          "action: Skip on Skip with a different color");
    check(!valid(act(CardColor::Blue, CardType::Reverse), skipTop,
                 CardColor::Red, hand, penalty),
          "action: Reverse on Skip with a different color is invalid");
    check(valid(act(CardColor::Red, CardType::Reverse), skipTop,
                CardColor::Red, hand, penalty),
          "action: Reverse on Skip with the same color");

    Card reverseTop = act(CardColor::Blue, CardType::Reverse);
    check(valid(act(CardColor::Green, CardType::Reverse), reverseTop,
                CardColor::Blue, hand, penalty),
          "action: Reverse on Reverse with a different color");

    Card drawTwoTop = act(CardColor::Yellow, CardType::DrawTwo);
    check(valid(act(CardColor::Green, CardType::DrawTwo), drawTwoTop,
                CardColor::Yellow, hand, penalty),
          "action: DrawTwo on DrawTwo with a different color");
    check(!valid(num(CardColor::Green, 2), drawTwoTop,
                 CardColor::Yellow, hand, penalty),
          "action: number does not match an action card by itself");
}

static void testWildCards() {
    SinglyLinkedList<Card> hand;
    CustomStack<int> penalty;
    Card top = num(CardColor::Red, 5);

    check(valid(wild(), top, CardColor::Red, hand, penalty),
          "wild: always valid");
    check(valid(wild(), act(CardColor::Blue, CardType::DrawTwo),
                CardColor::Blue, hand, penalty),
          "wild: valid on an action card");

    hand.addLast(num(CardColor::Red, 3));
    check(valid(wild(), top, CardColor::Red, hand, penalty),
          "wild: valid even with an active-color card in hand");

    Card wildTop = wild();
    CardColor chosen = CardColor::Green;
    SinglyLinkedList<Card> emptyHand;

    check(valid(num(CardColor::Green, 3), wildTop, chosen, emptyHand, penalty),
          "after wild: card of the chosen color is valid");
    check(!valid(num(CardColor::Red, 3), wildTop, chosen, emptyHand, penalty),
          "after wild: card of another color is invalid");
    check(!valid(act(CardColor::Red, CardType::Skip), wildTop, chosen,
                 emptyHand, penalty),
          "after wild: action card of another color is invalid");
    check(valid(act(CardColor::Green, CardType::Skip), wildTop, chosen,
                emptyHand, penalty),
          "after wild: action card of the chosen color is valid");
}

static void testWildDrawFourRestriction() {
    CustomStack<int> penalty;
    Card top = num(CardColor::Red, 5);

    SinglyLinkedList<Card> noRed;
    noRed.addLast(num(CardColor::Blue, 1));
    noRed.addLast(act(CardColor::Green, CardType::Skip));
    noRed.addLast(wildDrawFour());
    check(valid(wildDrawFour(), top, CardColor::Red, noRed, penalty),
          "wd4: valid when the hand has no card of the active color");

    SinglyLinkedList<Card> withRed;
    withRed.addLast(wildDrawFour());
    withRed.addLast(num(CardColor::Red, 8));
    check(!valid(wildDrawFour(), top, CardColor::Red, withRed, penalty),
          "wd4: invalid when the hand has a card of the active color");

    SinglyLinkedList<Card> redAction;
    redAction.addLast(act(CardColor::Red, CardType::Reverse));
    check(!valid(wildDrawFour(), top, CardColor::Red, redAction, penalty),
          "wd4: an action card of the active color also blocks it");

    SinglyLinkedList<Card> onlyWilds;
    onlyWilds.addLast(wild());
    onlyWilds.addLast(wildDrawFour());
    check(valid(wildDrawFour(), top, CardColor::Red, onlyWilds, penalty),
          "wd4: wild cards in hand do not block it");

    SinglyLinkedList<Card> emptyHand;
    check(valid(wildDrawFour(), top, CardColor::Red, emptyHand, penalty),
          "wd4: valid with an empty hand");
}

static void testPenaltyMode() {
    SinglyLinkedList<Card> hand;
    hand.addLast(num(CardColor::Red, 1));

    CustomStack<int> penalty;
    penalty.push(2);

    Card top = act(CardColor::Red, CardType::DrawTwo);
    CardColor active = CardColor::Red;

    check(valid(act(CardColor::Blue, CardType::DrawTwo), top, active, hand, penalty),
          "penalty: DrawTwo of any color is valid");
    check(valid(wildDrawFour(), top, active, hand, penalty),
          "penalty: WildDrawFour is valid even with an active-color card in hand");
    check(!valid(num(CardColor::Red, 7), top, active, hand, penalty),
          "penalty: number card of the same color is invalid");
    check(!valid(act(CardColor::Red, CardType::Skip), top, active, hand, penalty),
          "penalty: Skip is invalid");
    check(!valid(act(CardColor::Red, CardType::Reverse), top, active, hand, penalty),
          "penalty: Reverse is invalid");
    check(!valid(wild(), top, active, hand, penalty),
          "penalty: plain Wild is invalid");

    CustomStack<int> bigPenalty;
    bigPenalty.push(4);
    bigPenalty.push(2);
    check(valid(act(CardColor::Green, CardType::DrawTwo), top, active, hand, bigPenalty),
          "penalty: still works with several stacked penalties");

    CustomStack<int> noPenalty;
    check(valid(num(CardColor::Red, 7), top, active, hand, noPenalty),
          "penalty: back to normal rules when the penalty is cleared");
}

static void testPenaltyValue() {
    check(RuleManager::penaltyValue(act(CardColor::Red, CardType::DrawTwo)) == 2,
          "penaltyValue: DrawTwo is 2");
    check(RuleManager::penaltyValue(wildDrawFour()) == 4,
          "penaltyValue: WildDrawFour is 4");
    check(RuleManager::penaltyValue(wild()) == 0 &&
          RuleManager::penaltyValue(num(CardColor::Red, 2)) == 0 &&
          RuleManager::penaltyValue(act(CardColor::Red, CardType::Skip)) == 0 &&
          RuleManager::penaltyValue(act(CardColor::Red, CardType::Reverse)) == 0,
          "penaltyValue: other cards are 0");
}

int main() {
    cout << "--- Color and number matching ---\n";
    testColorAndNumber();

    cout << "\n--- Action card matching ---\n";
    testActionTypes();

    cout << "\n--- Wild cards ---\n";
    testWildCards();

    cout << "\n--- WildDrawFour restriction ---\n";
    testWildDrawFourRestriction();

    cout << "\n--- Penalty mode ---\n";
    testPenaltyMode();

    cout << "\n--- penaltyValue ---\n";
    testPenaltyValue();

    cout << "\nFailed checks: " << failedChecks << '\n';
    return failedChecks == 0 ? 0 : 1;
}