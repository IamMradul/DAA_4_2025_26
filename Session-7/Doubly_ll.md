```

#include <bits/stdc++.h>
using namespace std;

class Node {
public:
    int val;
    Node* next;
    Node* prev;

    Node(int data) {
        val = data;
        next = prev = NULL;
    }
};

class DoublyLinkedList {
    Node* head;
    Node* tail;

public:
    DoublyLinkedList() {
        head = tail = NULL;
    }
    void insert_begin(int val) {
        Node* newNode = new Node(val);

        if (head == NULL) {
            head = tail = newNode;
            return;
        }
        newNode->next = head;
        head->prev = newNode;
        head = newNode;
    }
    void insert_end(int val) {
        Node* newNode = new Node(val);
        if (tail == NULL) {
            head = tail = newNode;
            return;
        }
        tail->next = newNode;
        newNode->prev = tail;
        tail = newNode;
    }

    void insert_at_index(int index, int val) {
        if (index == 0) {
            insert_begin(val);
            return;
        }
        Node* temp = head;
        int count = 0;
        while (temp != NULL && count < index - 1) {
            temp = temp->next;
            count++;
        }
        if (temp == NULL) {
            cout << "Index out of bounds\n";
            return;
        }
        if (temp->next == NULL) {
            insert_end(val);
            return;
        }
        Node* newNode = new Node(val);
        newNode->next = temp->next;
        newNode->prev = temp;
        temp->next->prev = newNode;
        temp->next = newNode;
    }

    void delete_begin() {
        if (head == NULL) return;
        Node* temp = head;
        if (head == tail) {
            head = tail = NULL;
        } else {
            head = head->next;
            head->prev = NULL;
        }
        delete temp;
    }

    void delete_end() {
        if (tail == NULL) return;
        Node* temp = tail;
        if (head == tail) {
            head = tail = NULL;
        } else {
            tail = tail->prev;
            tail->next = NULL;
        }
        delete temp;
    }

    void delete_value(int val) {
        Node* temp = head;
        while (temp != NULL) {
            if (temp->val == val) {
                if (temp == head) {
                    delete_begin();
                } else if (temp == tail) {
                    delete_end();
                } else {
                    temp->prev->next = temp->next;
                    temp->next->prev = temp->prev;
                    delete temp;
                }
                return;
            }
            temp = temp->next;
        }
        cout << "Value not found\n";
    }

    void reverse() {
        Node* curr = head;
        Node* temp = NULL;
        while (curr != NULL) {
            temp = curr->prev;
            curr->prev = curr->next;
            curr->next = temp;
            curr = curr->prev;
        }
        if (temp != NULL) {
            head = temp->prev;
        }
    }

    void display() {
        Node* temp = head;
        while (temp != NULL) {
            cout << temp->val << " <-> ";
            temp = temp->next;
        }
        cout << "NULL\n";
    }

    void display_reverse() {
        Node* temp = tail;
        while (temp != NULL) {
            cout << temp->val << " <-> ";
            temp = temp->prev;
        }
        cout << "NULL\n";
    }
};

int main() {
    DoublyLinkedList dll;

    dll.insert_begin(3);
    dll.insert_begin(2);
    dll.insert_begin(1);
    dll.insert_end(4);
    dll.insert_end(5);
    dll.display();
    dll.insert_at_index(2, 99);
    dll.display();
    dll.delete_value(99);
    dll.display();
    dll.reverse();
    dll.display();

    return 0;
}

```