#include <iostream>

#include "../src/logic/Recursion.h"
#include "../src/ds/SinglyLinkedList.h"

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

static bool ascending(const int& a, const int& b) {
    return a < b;
}

static bool descending(const int& a, const int& b) {
    return a > b;
}

static bool arrayEquals(const DynamicArray<int>& array,
                        const int expected[], int length) {
    if (array.size() != length) {
        return false;
    }

    for (int i = 0; i < length; ++i) {
        int value = 0;

        if (!array.get(i, value) || value != expected[i]) {
            return false;
        }
    }

    return true;
}

struct Entry {
    int key;
    int id;

    Entry()
        : key(0), id(0) {}

    Entry(int entryKey, int entryId)
        : key(entryKey), id(entryId) {}
};

static bool byKey(const Entry& a, const Entry& b) {
    return a.key < b.key;
}

static void testSumPenalty() {
    CustomStack<int> stack;

    check(sumPenalty(stack) == 0, "sumPenalty: empty stack gives 0");

    stack.push(2);
    check(sumPenalty(stack) == 2 && stack.isEmpty(),
          "sumPenalty: single value, stack is emptied");

    stack.push(2);
    stack.push(4);
    stack.push(4);
    check(sumPenalty(stack) == 10 && stack.isEmpty(),
          "sumPenalty: 2 + 4 + 4 = 10, stack is emptied");

    for (int i = 0; i < 50; ++i) {
        stack.push(2);
    }
    check(sumPenalty(stack) == 100 && stack.isEmpty(),
          "sumPenalty: long chain of 50 values");
}

static void testCalcHandScore() {
    check(calcHandScore(nullptr) == 0, "calcHandScore: null gives 0");

    SinglyLinkedList<Card> hand;
    check(calcHandScore(hand.getHead()) == 0,
          "calcHandScore: empty hand gives 0");

    hand.addLast(Card(CardColor::Red, 7));
    check(calcHandScore(hand.getHead()) == 7,
          "calcHandScore: number card is worth its number");

    hand.addLast(Card(CardColor::Blue, CardType::Skip));
    check(calcHandScore(hand.getHead()) == 27,
          "calcHandScore: action card adds 20");

    hand.addLast(Card(CardColor::None, CardType::Wild));
    check(calcHandScore(hand.getHead()) == 77,
          "calcHandScore: wild card adds 50");

    hand.addLast(Card(CardColor::Green, 0));
    hand.addLast(Card(CardColor::Yellow, 9));
    check(calcHandScore(hand.getHead()) == 86,
          "calcHandScore: zero adds 0 and nine adds 9");

    DynamicArray<Card> standard;
    buildStandardDeck(standard);

    SinglyLinkedList<Card> fullDeck;
    for (int i = 0; i < standard.size(); ++i) {
        Card card;
        standard.get(i, card);
        fullDeck.addLast(card);
    }
    check(calcHandScore(fullDeck.getHead()) == 1240,
          "calcHandScore: all 108 cards add up to 1240");
}

static void testMergeSort() {
    DynamicArray<int> array;

    recursiveMergeSort(array, ascending);
    check(array.isEmpty(), "sort: empty array");

    array.add(42);
    recursiveMergeSort(array, ascending);
    const int single[] = {42};
    check(arrayEquals(array, single, 1), "sort: single element");

    DynamicArray<int> sorted;
    for (int i = 1; i <= 5; ++i) {
        sorted.add(i);
    }
    recursiveMergeSort(sorted, ascending);
    const int sortedExpected[] = {1, 2, 3, 4, 5};
    check(arrayEquals(sorted, sortedExpected, 5), "sort: already sorted");

    DynamicArray<int> reversed;
    for (int i = 5; i >= 1; --i) {
        reversed.add(i);
    }
    recursiveMergeSort(reversed, ascending);
    check(arrayEquals(reversed, sortedExpected, 5), "sort: reversed input");

    DynamicArray<int> duplicates;
    const int dupInput[] = {3, 1, 2, 3, 1, 2};
    for (int i = 0; i < 6; ++i) {
        duplicates.add(dupInput[i]);
    }
    recursiveMergeSort(duplicates, ascending);
    const int dupExpected[] = {1, 1, 2, 2, 3, 3};
    check(arrayEquals(duplicates, dupExpected, 6), "sort: duplicates");

    recursiveMergeSort(duplicates, descending);
    const int dupDescending[] = {3, 3, 2, 2, 1, 1};
    check(arrayEquals(duplicates, dupDescending, 6),
          "sort: descending comparator");

    DynamicArray<int> big;
    for (int i = 0; i < 1000; ++i) {
        big.add((i * 7919) % 1000);
    }
    recursiveMergeSort(big, ascending);

    bool bigOk = big.size() == 1000;
    for (int i = 0; i < 1000; ++i) {
        int value = -1;
        if (!big.get(i, value) || value != i) {
            bigOk = false;
        }
    }
    check(bigOk, "sort: 1000 shuffled values end up in order");

    DynamicArray<Entry> entries;
    for (int i = 0; i < 20; ++i) {
        entries.add(Entry(i % 3, i));
    }
    recursiveMergeSort(entries, byKey);

    bool keysOk = true;
    bool stableOk = true;
    for (int i = 1; i < entries.size(); ++i) {
        Entry previous;
        Entry current;
        entries.get(i - 1, previous);
        entries.get(i, current);

        if (previous.key > current.key) {
            keysOk = false;
        }

        if (previous.key == current.key && previous.id > current.id) {
            stableOk = false;
        }
    }
    check(entries.size() == 20 && keysOk, "sort: structs are ordered by key");
    check(stableOk, "sort: equal keys keep their original order (stable)");
}

int main() {
    cout << "--- sumPenalty tests ---\n";
    testSumPenalty();

    cout << "\n--- calcHandScore tests ---\n";
    testCalcHandScore();

    cout << "\n--- recursiveMergeSort tests ---\n";
    testMergeSort();

    cout << "\nFailed checks: " << failedChecks << '\n';
    return failedChecks == 0 ? 0 : 1;
}