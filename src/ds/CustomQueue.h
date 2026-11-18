
#ifndef CUSTOM_QUEUE_H
#define CUSTOM_QUEUE_H

#include "Node.h"

template <typename T>
class CustomQueue {
private:
    Node<T>* frontNode;
    Node<T>* rearNode;
    int count;

public:
    CustomQueue()
        : frontNode(nullptr),
          rearNode(nullptr),
          count(0) {}

    ~CustomQueue() {
        clear();
    }

    CustomQueue(const CustomQueue&) = delete;
    CustomQueue& operator=(const CustomQueue&) = delete;

    bool isEmpty() const {
        return frontNode == nullptr;
    }

    int size() const {
        return count;
    }

    void enqueue(const T& value) {
        Node<T>* node = new Node<T>(value);

        if (isEmpty()) {
            frontNode = node;
            rearNode = node;
        } else {
            rearNode->next = node;
            rearNode = node;
        }

        ++count;
    }

    bool dequeue(T& result) {
        if (isEmpty()) {
            return false;
        }

        Node<T>* temp = frontNode;
        result = temp->data;
        frontNode = frontNode->next;

        delete temp;
        --count;

        if (frontNode == nullptr) {
            rearNode = nullptr;
        }

        return true;
    }

    bool peek(T& result) const {
        if (isEmpty()) {
            return false;
        }

        result = frontNode->data;
        return true;
    }

    void clear() {
        while (frontNode != nullptr) {
            Node<T>* temp = frontNode;
            frontNode = frontNode->next;
            delete temp;
        }

        rearNode = nullptr;
        count = 0;
    }
};

#endif
