#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

template <typename T>
class DynamicArray {
private:
    T* data;
    int count;
    int capacity;

    void resize(int newCapacity) {
        T* newData = new T[newCapacity];

        for (int i = 0; i < count; ++i) {
            newData[i] = data[i];
        }

        delete[] data;
        data = newData;
        capacity = newCapacity;
    }

public:
    DynamicArray()
        : data(new T[10]), count(0), capacity(10) {}

    ~DynamicArray() {
        delete[] data;
    }

    DynamicArray(const DynamicArray&) = delete;
    DynamicArray& operator=(const DynamicArray&) = delete;

    int size() const {
        return count;
    }

    int getCapacity() const {
        return capacity;
    }

    bool isEmpty() const {
        return count == 0;
    }

    void add(const T& value) {
        if (count == capacity) {
            resize(capacity * 2);
        }

        data[count] = value;
        ++count;
    }

    bool get(int index, T& result) const {
        if (index < 0 || index >= count) {
            return false;
        }

        result = data[index];
        return true;
    }

    bool set(int index, const T& value) {
        if (index < 0 || index >= count) {
            return false;
        }

        data[index] = value;
        return true;
    }

    bool swap(int first, int second) {
        if (first < 0 || first >= count ||
            second < 0 || second >= count) {
            return false;
        }

        T temp = data[first];
        data[first] = data[second];
        data[second] = temp;

        return true;
    }

    bool removeLast() {
        if (count == 0) {
            return false;
        }

        --count;
        return true;
    }
};

#endif