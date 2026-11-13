#ifndef RECURSION_H
#define RECURSION_H

#include "../ds/CustomStack.h"
#include "../ds/DynamicArray.h"
#include "../ds/Node.h"
#include "../model/Card.h"

using namespace std;

inline int sumPenalty(CustomStack<int>& penaltyStack) {
    if (penaltyStack.isEmpty()) {
        return 0;
    }

    int value = 0;
    penaltyStack.pop(value);

    return value + sumPenalty(penaltyStack);
}

inline int calcHandScore(Node<Card>* node) {
    if (node == nullptr) {
        return 0;
    }

    return node->data.getValue() + calcHandScore(node->next);
}

template <typename T>
T valueAt(const DynamicArray<T>& array, int index) {
    T value;
    array.get(index, value);
    return value;
}

template <typename T, typename Compare>
void mergeStep(const DynamicArray<T>& array, DynamicArray<T>& temp,
               int i, int mid, int j, int right, Compare cmp) {
    if (i >= mid && j >= right) {
        return;
    }

    bool takeLeft;

    if (i >= mid) {
        takeLeft = false;
    } else if (j >= right) {
        takeLeft = true;
    } else {
        takeLeft = !cmp(valueAt(array, j), valueAt(array, i));
    }

    if (takeLeft) {
        temp.add(valueAt(array, i));
        mergeStep(array, temp, i + 1, mid, j, right, cmp);
    } else {
        temp.add(valueAt(array, j));
        mergeStep(array, temp, i, mid, j + 1, right, cmp);
    }
}

template <typename T>
void copyBack(DynamicArray<T>& array, const DynamicArray<T>& temp,
              int start, int offset) {
    if (offset >= temp.size()) {
        return;
    }

    array.set(start + offset, valueAt(temp, offset));
    copyBack(array, temp, start, offset + 1);
}

template <typename T, typename Compare>
void sortRange(DynamicArray<T>& array, int left, int right, Compare cmp) {
    if (right - left < 2) {
        return;
    }

    int mid = left + (right - left) / 2;

    sortRange(array, left, mid, cmp);
    sortRange(array, mid, right, cmp);

    DynamicArray<T> temp;
    mergeStep(array, temp, left, mid, mid, right, cmp);
    copyBack(array, temp, left, 0);
}

template <typename T, typename Compare>
void recursiveMergeSort(DynamicArray<T>& array, Compare cmp) {
    sortRange(array, 0, array.size(), cmp);
}

#endif