#ifndef RANDOM_H
#define RANDOM_H

#include <cstdlib>
#include <ctime>

using namespace std;

inline void seedRandom() {
    srand(static_cast<unsigned int>(time(nullptr)));
}

inline int randomInt(int maxExclusive) {
    if (maxExclusive <= 0) {
        return 0;
    }

    return rand() % maxExclusive;
}

#endif