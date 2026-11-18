
#ifndef CUSTOM_STACK_H
#define CUSTOM_STACK_H

#include "Node.h"

template <typename T>
class CustomStack {
private:
    Node<T>* topNode;
    int count;

public:
    CustomStack()
        : topNode(nullptr), count(0) {}

    ~CustomStack() {
        clear();
    }

    CustomStack(const CustomStack&) = delete;
    CustomStack& operator=(const CustomStack&) = delete;

    bool isEmpty() const {
        return topNode == nullptr;
    }

    int size() const {
        return count;
    }

    void push(const T& value) {
        Node<T>* node = new Node<T>(value);
        node->next = topNode;
        topNode = node;
        ++count;
    }

    bool pop(T& result) {
        if (isEmpty()) {
            return false;
        }

        Node<T>* temp = topNode;
        result = temp->data;
        topNode = topNode->next;

        delete temp;
        --count;

        return true;
    }

    bool peek(T& result) const {
        if (isEmpty()) {
            return false;
        }

        result = topNode->data;
        return true;
    }

    void clear() {
        while (topNode != nullptr) {
            Node<T>* temp = topNode;
            topNode = topNode->next;
            delete temp;
        }

        count = 0;
    }
};

#endif
