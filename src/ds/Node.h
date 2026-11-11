#ifndef NODE_H
#define NODE_H

template <typename T>
struct Node {
    T data;
    Node<T>* next;

    Node(const T& value)
        : data(value), next(nullptr) {}
};

template <typename T>
struct DoublyNode {
    T data;
    DoublyNode<T>* next;
    DoublyNode<T>* prev;

    DoublyNode(const T& value)
        : data(value), next(nullptr), prev(nullptr) {}
};

#endif