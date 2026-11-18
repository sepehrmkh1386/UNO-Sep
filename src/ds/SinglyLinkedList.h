#ifndef SINGLY_LINKED_LIST_H
#define SINGLY_LINKED_LIST_H

#include "Node.h"

template <typename T>
class SinglyLinkedList {
private:
    Node<T>* head;
    Node<T>* tail;
    int count;

    static Node<T>* splitMiddle(Node<T>* first) {
        Node<T>* slow = first;
        Node<T>* fast = first->next;

        while (fast != nullptr && fast->next != nullptr) {
            slow = slow->next;
            fast = fast->next->next;
        }

        Node<T>* second = slow->next;
        slow->next = nullptr;

        return second;
    }

    template <typename Compare>
    static Node<T>* mergeNodes(Node<T>* left, Node<T>* right, Compare cmp) {
        if (left == nullptr) {
            return right;
        }

        if (right == nullptr) {
            return left;
        }

        if (cmp(right->data, left->data)) {
            right->next = mergeNodes(left, right->next, cmp);
            return right;
        }

        left->next = mergeNodes(left->next, right, cmp);
        return left;
    }

    template <typename Compare>
    static Node<T>* mergeSortNodes(Node<T>* first, Compare cmp) {
        if (first == nullptr || first->next == nullptr) {
            return first;
        }

        Node<T>* second = splitMiddle(first);

        first = mergeSortNodes(first, cmp);
        second = mergeSortNodes(second, cmp);

        return mergeNodes(first, second, cmp);
    }

public:
    SinglyLinkedList()
        : head(nullptr), tail(nullptr), count(0) {}

    ~SinglyLinkedList() {
        clear();
    }

    SinglyLinkedList(const SinglyLinkedList&) = delete;
    SinglyLinkedList& operator=(const SinglyLinkedList&) = delete;

    bool isEmpty() const {
        return head == nullptr;
    }

    int size() const {
        return count;
    }

    Node<T>* getHead() const {
        return head;
    }

    void prepend(const T& value) {
        Node<T>* node = new Node<T>(value);
        node->next = head;
        head = node;

        if (tail == nullptr) {
            tail = node;
        }

        ++count;
    }

    void addLast(const T& value) {
        Node<T>* node = new Node<T>(value);

        if (tail == nullptr) {
            head = node;
        } else {
            tail->next = node;
        }

        tail = node;
        ++count;
    }

    bool insertAt(int index, const T& value) {
        if (index < 0 || index > count) {
            return false;
        }

        if (index == 0) {
            prepend(value);
            return true;
        }

        if (index == count) {
            addLast(value);
            return true;
        }

        Node<T>* current = head;

        for (int i = 0; i < index - 1; ++i) {
            current = current->next;
        }

        Node<T>* node = new Node<T>(value);
        node->next = current->next;
        current->next = node;
        ++count;

        return true;
    }

    bool get(int index, T& result) const {
        if (index < 0 || index >= count) {
            return false;
        }

        Node<T>* current = head;

        for (int i = 0; i < index; ++i) {
            current = current->next;
        }

        result = current->data;
        return true;
    }

    bool removeAt(int index, T& removed) {
        if (index < 0 || index >= count) {
            return false;
        }

        Node<T>* target;

        if (index == 0) {
            target = head;
            head = head->next;

            if (head == nullptr) {
                tail = nullptr;
            }
        } else {
            Node<T>* previous = head;

            for (int i = 0; i < index - 1; ++i) {
                previous = previous->next;
            }

            target = previous->next;
            previous->next = target->next;

            if (target == tail) {
                tail = previous;
            }
        }

        removed = target->data;
        delete target;
        --count;

        return true;
    }

    bool find(const T& value, int& index) const {
        Node<T>* current = head;
        int position = 0;

        while (current != nullptr) {
            if (current->data == value) {
                index = position;
                return true;
            }

            current = current->next;
            ++position;
        }

        return false;
    }

    template <typename Compare>
    void sort(Compare cmp) {
        head = mergeSortNodes(head, cmp);

        tail = head;

        if (tail != nullptr) {
            while (tail->next != nullptr) {
                tail = tail->next;
            }
        }
    }

    void clear() {
        while (head != nullptr) {
            Node<T>* temp = head;
            head = head->next;
            delete temp;
        }

        tail = nullptr;
        count = 0;
    }
};

#endif