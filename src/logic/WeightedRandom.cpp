#include "WeightedRandom.h"
#include "Random.h"

static const int CHANCE_OPTION_COUNT = 4;
static const int CHANCE_WEIGHTS[CHANCE_OPTION_COUNT] = {1, 2, 2, 1};
static const int PLUS_TWO_CARDS[CHANCE_OPTION_COUNT] = {0, 1, 2, 4};
static const int PLUS_FOUR_CARDS[CHANCE_OPTION_COUNT] = {0, 2, 4, 6};

WeightedRandom::WeightedRandom(const int weightValues[], int length)
    : total(0), ready(false) {
    if (weightValues == nullptr || length <= 0) {
        return;
    }

    int sum = 0;

    for (int i = 0; i < length; ++i) {
        if (weightValues[i] < 0) {
            return;
        }

        sum += weightValues[i];
    }

    if (sum <= 0) {
        return;
    }

    int running = 0;

    for (int i = 0; i < length; ++i) {
        running += weightValues[i];
        weights.add(weightValues[i]);
        prefixSums.add(running);
    }

    total = sum;
    ready = true;
}

bool WeightedRandom::isReady() const {
    return ready;
}

int WeightedRandom::size() const {
    return weights.size();
}

int WeightedRandom::totalWeight() const {
    return total;
}

int WeightedRandom::prefixAt(int index) const {
    int value = -1;

    if (!prefixSums.get(index, value)) {
        return -1;
    }

    return value;
}

int WeightedRandom::indexForRoll(int roll) const {
    if (!ready || roll < 0 || roll >= total) {
        return -1;
    }

    for (int i = 0; i < prefixSums.size(); ++i) {
        int prefix = 0;
        prefixSums.get(i, prefix);

        if (roll < prefix) {
            return i;
        }
    }

    return -1;
}

int WeightedRandom::pick() const {
    if (!ready) {
        return -1;
    }

    return indexForRoll(randomInt(total));
}

ChanceMachine::ChanceMachine()
    : plusTwoMachine(CHANCE_WEIGHTS, CHANCE_OPTION_COUNT),
      plusFourMachine(CHANCE_WEIGHTS, CHANCE_OPTION_COUNT) {
}

int ChanceMachine::totalWeight(bool isPlusFour) const {
    if (isPlusFour) {
        return plusFourMachine.totalWeight();
    }

    return plusTwoMachine.totalWeight();
}

int ChanceMachine::spinWithRoll(bool isPlusFour, int roll) const {
    const WeightedRandom& machine =
        isPlusFour ? plusFourMachine : plusTwoMachine;
    const int* cards = isPlusFour ? PLUS_FOUR_CARDS : PLUS_TWO_CARDS;

    int index = machine.indexForRoll(roll);

    if (index < 0) {
        return -1;
    }

    return cards[index];
}

int ChanceMachine::spin(bool isPlusFour) const {
    const WeightedRandom& machine =
        isPlusFour ? plusFourMachine : plusTwoMachine;
    const int* cards = isPlusFour ? PLUS_FOUR_CARDS : PLUS_TWO_CARDS;

    int index = machine.pick();

    if (index < 0) {
        return -1;
    }

    return cards[index];
}