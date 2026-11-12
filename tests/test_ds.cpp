#include <iostream>

#include "../src/ds/SinglyLinkedList.h"
#include "../src/ds/CustomStack.h"
#include "../src/ds/CustomQueue.h"
#include "../src/ds/DynamicArray.h"
#include "../src/ds/DoublyCircularLinkedList.h"

using namespace std;

static int failedChecks = 0;

static void check(bool condition, const char* name) {
    if (condition) {
        cout << "[PASS] " << name << '\n';
    } else {
        cout << "[FAIL] " << name << '\n';
        ++failedChecks;
    }
}

static bool ascending(const int& a, const int& b) {
    return a < b;
}

static bool descending(const int& a, const int& b) {
    return a > b;
}

static bool listEquals(const SinglyLinkedList<int>& list,
                       const int expected[], int length) {
    if (list.size() != length) {
        return false;
    }

    for (int i = 0; i < length; ++i) {
        int value = 0;

        if (!list.get(i, value) || value != expected[i]) {
            return false;
        }
    }

    return true;
}

static void testSinglyLinkedList() {
    SinglyLinkedList<int> list;
    int value = 0;

    check(list.isEmpty() && list.size() == 0 && list.getHead() == nullptr,
          "list: starts empty");

    list.addLast(10);
    list.addLast(20);
    list.addLast(30);
    const int afterAdd[] = {10, 20, 30};
    check(listEquals(list, afterAdd, 3), "list: addLast keeps order");
    check(list.getHead() != nullptr && list.getHead()->data == 10,
          "list: getHead returns first node");

    list.prepend(5);
    const int afterPrepend[] = {5, 10, 20, 30};
    check(listEquals(list, afterPrepend, 4), "list: prepend adds to front");

    check(list.removeAt(3, value) && value == 30, "list: remove last element");
    list.addLast(40);
    const int afterRemoveLast[] = {5, 10, 20, 40};
    check(listEquals(list, afterRemoveLast, 4),
          "list: addLast works after removing last (tail updated)");

    check(list.removeAt(0, value) && value == 5, "list: remove first element");
    check(!list.removeAt(99, value), "list: removeAt invalid index fails");
    check(!list.get(-1, value), "list: get invalid index fails");

    check(list.insertAt(3, 50), "list: insertAt end succeeds");
    list.addLast(60);
    const int afterInsertEnd[] = {10, 20, 40, 50, 60};
    check(listEquals(list, afterInsertEnd, 5),
          "list: addLast works after insertAt end (tail updated)");

    list.sort(descending);
    const int descendingOrder[] = {60, 50, 40, 20, 10};
    check(listEquals(list, descendingOrder, 5), "list: sort descending");

    list.sort(ascending);
    const int ascendingOrder[] = {10, 20, 40, 50, 60};
    check(listEquals(list, ascendingOrder, 5), "list: sort ascending");

    list.addLast(70);
    const int afterSortAdd[] = {10, 20, 40, 50, 60, 70};
    check(listEquals(list, afterSortAdd, 6),
          "list: addLast works after sort (tail updated)");

    SinglyLinkedList<int> emptyList;
    emptyList.sort(ascending);
    check(emptyList.isEmpty(), "list: sort on empty list");

    SinglyLinkedList<int> singleList;
    singleList.addLast(1);
    singleList.sort(ascending);
    check(singleList.size() == 1 && singleList.get(0, value) && value == 1,
          "list: sort on single element");

    list.clear();
    check(list.isEmpty() && list.size() == 0, "list: clear empties the list");
    list.addLast(1);
    check(list.size() == 1 && list.get(0, value) && value == 1,
          "list: addLast works after clear");
}

static void testCustomStack() {
    CustomStack<int> stack;
    int value = 0;

    check(stack.isEmpty() && stack.size() == 0, "stack: starts empty");
    check(!stack.pop(value), "stack: pop on empty fails");
    check(!stack.peek(value), "stack: peek on empty fails");

    stack.push(1);
    stack.push(2);
    stack.push(3);
    check(stack.peek(value) && value == 3 && stack.size() == 3,
          "stack: peek returns top without removing");

    bool order = stack.pop(value) && value == 3;
    order = order && stack.pop(value) && value == 2;
    order = order && stack.pop(value) && value == 1;
    check(order, "stack: pop follows LIFO order");
    check(stack.isEmpty() && stack.size() == 0,
          "stack: empty after popping everything");

    stack.push(9);
    check(stack.peek(value) && value == 9 && stack.size() == 1,
          "stack: push works after becoming empty");

    stack.push(10);
    stack.clear();
    check(stack.isEmpty() && stack.size() == 0, "stack: clear empties the stack");
}

static void testCustomQueue() {
    CustomQueue<int> queue;
    int value = 0;

    check(queue.isEmpty() && queue.size() == 0, "queue: starts empty");
    check(!queue.dequeue(value), "queue: dequeue on empty fails");
    check(!queue.peek(value), "queue: peek on empty fails");

    queue.enqueue(1);
    queue.enqueue(2);
    queue.enqueue(3);
    check(queue.peek(value) && value == 1 && queue.size() == 3,
          "queue: peek returns front without removing");

    bool order = queue.dequeue(value) && value == 1;
    order = order && queue.dequeue(value) && value == 2;
    order = order && queue.dequeue(value) && value == 3;
    check(order, "queue: dequeue follows FIFO order");
    check(queue.isEmpty() && queue.size() == 0,
          "queue: empty after dequeuing everything");

    queue.enqueue(7);
    queue.dequeue(value);
    queue.enqueue(8);
    queue.enqueue(9);
    bool reuse = queue.dequeue(value) && value == 8;
    reuse = reuse && queue.dequeue(value) && value == 9;
    check(reuse, "queue: enqueue works after becoming empty (rear reset)");

    queue.enqueue(1);
    queue.enqueue(2);
    queue.clear();
    check(queue.isEmpty() && queue.size() == 0, "queue: clear empties the queue");
}

static void testDynamicArray() {
    DynamicArray<int> array;
    int value = 0;

    check(array.isEmpty() && array.size() == 0 && array.getCapacity() == 10,
          "array: starts empty with capacity 10");

    for (int i = 0; i < 1000; ++i) {
        array.add(i);
    }
    check(array.size() == 1000 && array.getCapacity() == 1280,
          "array: 1000 elements, capacity doubled to 1280");

    bool allValid = true;
    for (int i = 0; i < 1000; ++i) {
        if (!array.get(i, value) || value != i) {
            allValid = false;
        }
    }
    check(allValid, "array: all values preserved after resizing");

    check(!array.get(1000, value) && !array.get(-1, value),
          "array: get invalid index fails");

    check(array.set(5, 55) && array.get(5, value) && value == 55,
          "array: set changes a value");
    check(!array.set(1000, 1) && !array.set(-1, 1),
          "array: set invalid index fails");

    bool swapped = array.swap(0, 999);
    swapped = swapped && array.get(0, value) && value == 999;
    swapped = swapped && array.get(999, value) && value == 0;
    check(swapped, "array: swap exchanges two elements");
    check(!array.swap(0, 1000) && !array.swap(-1, 0),
          "array: swap invalid index fails");
}

static void testDoublyCircularList() {
    DoublyCircularLinkedList<int> list;
    int value = 0;

    check(list.isEmpty() && list.size() == 0 && list.getHead() == nullptr,
          "circular: starts empty");

    list.add(10);
    list.add(20);
    list.add(30);

    int first = 0;
    int second = 0;
    int third = 0;
    check(list.size() == 3 &&
          list.getAt(0, first) && list.getAt(1, second) &&
          list.getAt(2, third) &&
          first == 10 && second == 20 && third == 30,
          "circular: add keeps order");

    DoublyNode<int>* head = list.getHead();
    check(head != nullptr && head->next->next->next == head,
          "circular: forward links form a circle");
    check(head != nullptr && head->prev->prev->prev == head,
          "circular: backward links form a circle");

    check(list.getNext(2, value) && value == 10,
          "circular: getNext wraps to the start");
    check(list.getPrevious(0, value) && value == 30,
          "circular: getPrevious wraps to the end");

    bool removedMiddle = list.removeAt(1, value) && value == 20;
    head = list.getHead();
    check(removedMiddle && list.size() == 2 &&
          head->next->data == 30 && head->next->next == head &&
          head->prev == head->next,
          "circular: removeAt middle relinks neighbours");

    bool removedHead = list.removeAt(0, value) && value == 10;
    head = list.getHead();
    check(removedHead && list.size() == 1 && head != nullptr &&
          head->data == 30 && head->next == head && head->prev == head,
          "circular: removeAt head updates head");

    check(list.removeAt(0, value) && value == 30 &&
          list.isEmpty() && list.getHead() == nullptr,
          "circular: removing the last element empties the list");
    check(!list.removeAt(0, value) && !list.removeAt(-1, value),
          "circular: removeAt invalid index fails");

    list.add(1);
    list.add(2);
    list.add(3);
    list.clear();
    check(list.isEmpty() && list.getHead() == nullptr,
          "circular: clear empties the list");

    list.add(1);
    list.add(2);
    list.add(3);

    DoublyNode<int>* current = list.getHead();
    for (int i = 0; i < 4; ++i) {
        current = current->next;
    }
    bool forwardOk = current->data == 2;

    current = list.getHead();
    for (int i = 0; i < 4; ++i) {
        current = current->prev;
    }
    bool backwardOk = current->data == 3;

    check(forwardOk && backwardOk,
          "circular: pointer walk works in both directions");
}

int main() {
    SinglyLinkedList<int> list;
    list.addLast(10);
    list.addLast(20);

    int value = 0;
    list.get(1, value);
    cout << "List: " << value << '\n';

    CustomStack<int> stack;
    stack.push(10);
    stack.push(20);
    stack.pop(value);
    cout << "Stack: " << value << '\n';

    CustomQueue<int> queue;
    queue.enqueue(10);
    queue.enqueue(20);
    queue.dequeue(value);
    cout << "Queue: " << value << '\n';

    DynamicArray<int> array;
    for (int i = 0; i < 15; ++i) {
        array.add(i);
    }
    array.get(14, value);
    cout << "Array: " << value
         << ", capacity: " << array.getCapacity() << '\n';

    DoublyCircularLinkedList<int> circularList;
    circularList.add(10);
    circularList.add(20);
    circularList.add(30);
    circularList.getNext(2, value);
    cout << "Circular list: " << value << '\n';

    cout << "\n--- SinglyLinkedList tests ---\n";
    testSinglyLinkedList();

    cout << "\n--- CustomStack tests ---\n";
    testCustomStack();

    cout << "\n--- CustomQueue tests ---\n";
    testCustomQueue();

    cout << "\n--- DynamicArray tests ---\n";
    testDynamicArray();

    cout << "\n--- DoublyCircularLinkedList tests ---\n";
    testDoublyCircularList();

    cout << "\nFailed checks: " << failedChecks << '\n';
    return failedChecks == 0 ? 0 : 1;
}