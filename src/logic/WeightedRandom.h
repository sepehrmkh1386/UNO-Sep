#ifndef WEIGHTED_RANDOM_H
#define WEIGHTED_RANDOM_H

#include "../ds/DynamicArray.h"

class WeightedRandom {
private:
    DynamicArray<int> weights;
    DynamicArray<int> prefixSums;
    int total;
    bool ready;

public:
    WeightedRandom(const int weightValues[], int length);

    bool isReady() const;
    int size() const;
    int totalWeight() const;
    int prefixAt(int index) const;

    int indexForRoll(int roll) const;
    int pick() const;
};

class ChanceMachine {
private:
    WeightedRandom plusTwoMachine;
    WeightedRandom plusFourMachine;

public:
    ChanceMachine();

    int totalWeight(bool isPlusFour) const;
    int spinWithRoll(bool isPlusFour, int roll) const;
    int spin(bool isPlusFour) const;
};

#endif