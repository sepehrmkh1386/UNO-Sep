#include <iostream>

#include "../src/logic/WeightedRandom.h"

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

static void testInvalidWeights() {
    const int empty[] = {0};
    WeightedRandom noWeights(empty, 0);
    check(!noWeights.isReady() && noWeights.pick() == -1 &&
          noWeights.indexForRoll(0) == -1,
          "invalid: empty weight list is rejected");

    WeightedRandom nullWeights(nullptr, 3);
    check(!nullWeights.isReady() && nullWeights.pick() == -1,
          "invalid: null array is rejected");

    const int allZero[] = {0, 0, 0};
    WeightedRandom zeroSum(allZero, 3);
    check(!zeroSum.isReady() && zeroSum.totalWeight() == 0 &&
          zeroSum.pick() == -1,
          "invalid: all-zero weights are rejected (sum must not be zero)");

    const int negative[] = {3, -1, 2};
    WeightedRandom withNegative(negative, 3);
    check(!withNegative.isReady() && withNegative.size() == 0 &&
          withNegative.pick() == -1,
          "invalid: a negative weight is rejected");
}

static void testPrefixSums() {
    const int weights[] = {1, 2, 2};
    WeightedRandom machine(weights, 3);

    check(machine.isReady() && machine.size() == 3 &&
          machine.totalWeight() == 5,
          "prefix: sample weights 1,2,2 give total 5");
    check(machine.prefixAt(0) == 1 && machine.prefixAt(1) == 3 &&
          machine.prefixAt(2) == 5,
          "prefix: prefix sums are 1, 3, 5");
    check(machine.prefixAt(3) == -1 && machine.prefixAt(-1) == -1,
          "prefix: invalid index returns -1");
}

static void testRolls() {
    const int weights[] = {1, 2, 2};
    WeightedRandom machine(weights, 3);

    check(machine.indexForRoll(0) == 0, "roll: 0 selects option 0");
    check(machine.indexForRoll(1) == 1 && machine.indexForRoll(2) == 1,
          "roll: 1 and 2 select option 1");
    check(machine.indexForRoll(3) == 2 && machine.indexForRoll(4) == 2,
          "roll: 3 and 4 select option 2");
    check(machine.indexForRoll(5) == -1 && machine.indexForRoll(-1) == -1,
          "roll: out-of-range rolls are rejected");

    const int single[] = {7};
    WeightedRandom one(single, 1);
    check(one.indexForRoll(0) == 0 && one.indexForRoll(6) == 0 &&
          one.indexForRoll(7) == -1,
          "roll: a single option owns every roll");
}

static void testZeroWeights() {
    const int weights[] = {3, 0, 2, 0};
    WeightedRandom machine(weights, 4);

    bool byRoll = true;

    for (int roll = 0; roll < machine.totalWeight(); ++roll) {
        int index = machine.indexForRoll(roll);

        if (index != 0 && index != 2) {
            byRoll = false;
        }
    }
    check(machine.isReady() && machine.totalWeight() == 5 && byRoll,
          "zero weight: no roll ever selects a zero-weight option");

    bool byRandom = true;

    for (int i = 0; i < 5000; ++i) {
        int index = machine.pick();

        if (index != 0 && index != 2) {
            byRandom = false;
        }
    }
    check(byRandom, "zero weight: 5000 random picks never select it");

    const int leading[] = {0, 5};
    WeightedRandom leadingZero(leading, 2);
    bool leadingOk = true;

    for (int roll = 0; roll < 5; ++roll) {
        if (leadingZero.indexForRoll(roll) != 1) {
            leadingOk = false;
        }
    }
    check(leadingOk, "zero weight: a leading zero weight is never selected");
}

static void testDistribution() {
    const int weights[] = {1, 2, 2};
    WeightedRandom machine(weights, 3);

    int counts[3] = {0, 0, 0};
    const int trials = 10000;

    for (int i = 0; i < trials; ++i) {
        int index = machine.pick();

        if (index >= 0 && index < 3) {
            ++counts[index];
        }
    }

    check(counts[0] + counts[1] + counts[2] == trials,
          "distribution: every pick returns a valid option");
    check(counts[0] > 1600 && counts[0] < 2400,
          "distribution: weight 1 of 5 is picked about 20% of the time");
    check(counts[1] > 3600 && counts[1] < 4400,
          "distribution: weight 2 of 5 is picked about 40% of the time");
    check(counts[2] > 3600 && counts[2] < 4400,
          "distribution: second weight 2 of 5 is picked about 40% of the time");
}

static void testChanceMachineRolls() {
    ChanceMachine machine;

    check(machine.totalWeight(false) == 6 && machine.totalWeight(true) == 6,
          "chance: both machines have total weight 6");

    check(machine.spinWithRoll(false, 0) == 0 &&
          machine.spinWithRoll(false, 1) == 1 &&
          machine.spinWithRoll(false, 2) == 1 &&
          machine.spinWithRoll(false, 3) == 2 &&
          machine.spinWithRoll(false, 4) == 2 &&
          machine.spinWithRoll(false, 5) == 4,
          "chance: +2 machine maps rolls to 0, 1, 2 or 4 cards");

    check(machine.spinWithRoll(true, 0) == 0 &&
          machine.spinWithRoll(true, 1) == 2 &&
          machine.spinWithRoll(true, 2) == 2 &&
          machine.spinWithRoll(true, 3) == 4 &&
          machine.spinWithRoll(true, 4) == 4 &&
          machine.spinWithRoll(true, 5) == 6,
          "chance: +4 machine maps rolls to 0, 2, 4 or 6 cards");

    check(machine.spinWithRoll(false, 6) == -1 &&
          machine.spinWithRoll(true, -1) == -1,
          "chance: out-of-range rolls are rejected");
}

static void testChanceMachineSpin() {
    ChanceMachine machine;

    bool plusTwoOk = true;
    bool plusFourOk = true;
    int seenTwo[5] = {0, 0, 0, 0, 0};
    int seenFour[7] = {0, 0, 0, 0, 0, 0, 0};

    for (int i = 0; i < 6000; ++i) {
        int two = machine.spin(false);
        int four = machine.spin(true);

        if (two == 0 || two == 1 || two == 2 || two == 4) {
            ++seenTwo[two];
        } else {
            plusTwoOk = false;
        }

        if (four == 0 || four == 2 || four == 4 || four == 6) {
            ++seenFour[four];
        } else {
            plusFourOk = false;
        }
    }

    check(plusTwoOk, "chance: +2 spin only returns 0, 1, 2 or 4");
    check(plusFourOk, "chance: +4 spin only returns 0, 2, 4 or 6");
    check(seenTwo[0] > 0 && seenTwo[1] > 0 && seenTwo[2] > 0 && seenTwo[4] > 0,
          "chance: every +2 outcome can happen");
    check(seenFour[0] > 0 && seenFour[2] > 0 && seenFour[4] > 0 &&
          seenFour[6] > 0,
          "chance: every +4 outcome can happen");
    check(seenTwo[3] == 0 && seenFour[1] == 0 && seenFour[3] == 0 &&
          seenFour[5] == 0,
          "chance: impossible outcomes never happen");
}

int main() {
    cout << "--- Invalid weights ---\n";
    testInvalidWeights();

    cout << "\n--- Prefix sums ---\n";
    testPrefixSums();

    cout << "\n--- Rolls ---\n";
    testRolls();

    cout << "\n--- Zero weights ---\n";
    testZeroWeights();

    cout << "\n--- Distribution ---\n";
    testDistribution();

    cout << "\n--- ChanceMachine rolls ---\n";
    testChanceMachineRolls();

    cout << "\n--- ChanceMachine spin ---\n";
    testChanceMachineSpin();

    cout << "\nFailed checks: " << failedChecks << '\n';
    return failedChecks == 0 ? 0 : 1;
}