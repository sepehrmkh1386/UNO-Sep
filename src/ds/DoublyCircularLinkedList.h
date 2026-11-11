#ifndef DOUBLY_CIRCULAR_LINKED_LIST_H
#define DOUBLY_CIRCULAR_LINKED_LIST_H

#include "Node.h"

template <typename T>
class DoublyCircularLinkedList {
private:
    DoublyNode<T>* head;
    int count;

public:
    DoublyCircularLinkedList()
        : head(nullptr), count(0) {}

    ~DoublyCircularLinkedList() {
        clear();
    }

    DoublyCircularLinkedList(
        const DoublyCircularLinkedList&) = delete;

    DoublyCircularLinkedList& operator=(
        const DoublyCircularLinkedList&) = delete;

    bool isEmpty() const {
        return count == 0;
    }

    int size() const {
        return count;
    }

    DoublyNode<T>* getHead() const {
        return head;
    }

    bool add(const T& value) {
        DoublyNode<T>* node = new DoublyNode<T>(value);

        if (head == nullptr) {
            head = node;
            node->next = node;
            node->prev = node;
        } else {
            DoublyNode<T>* tail = head->prev;

            node->next = head;
            node->prev = tail;

            tail->next = node;
            head->prev = node;
        }

        ++count;
        return true;
    }

    bool getAt(int index, T& result) const {
        if (index < 0 || index >= count) {
            return false;
        }

        DoublyNode<T>* current = head;

        for (int i = 0; i < index; ++i) {
            current = current->next;
        }

        result = current->data;
        return true;
    }

    bool getNext(int index, T& result) const {
        if (index < 0 || index >= count) {
            return false;
        }

        DoublyNode<T>* current = head;

        for (int i = 0; i < index; ++i) {
            current = current->next;
        }

        result = current->next->data;
        return true;
    }

    bool getPrevious(int index, T& result) const {
        if (index < 0 || index >= count) {
            return false;
        }

        DoublyNode<T>* current = head;

        for (int i = 0; i < index; ++i) {
            current = current->next;
        }

        result = current->prev->data;
        return true;
    }

    bool removeAt(int index, T& removed) {
        if (index < 0 || index >= count) {
            return false;
        }

        DoublyNode<T>* target = head;

        for (int i = 0; i < index; ++i) {
            target = target->next;
        }

        removed = target->data;

        if (count == 1) {
            head = nullptr;
        } else {
            target->prev->next = target->next;
            target->next->prev = target->prev;

            if (target == head) {
                head = target->next;
            }
        }

        delete target;
        --count;

        return true;
    }

    void clear() {
        while (count > 0) {
            DoublyNode<T>* target = head;

            if (count == 1) {
                head = nullptr;
            } else {
                head->prev->next = head->next;
                head->next->prev = head->prev;
                head = head->next;
            }

            delete target;
            --count;
        }
    }
};

#endif