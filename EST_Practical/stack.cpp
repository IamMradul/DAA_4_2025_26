#include <iostream>
using namespace std;

class Node {
public:
    int data;
    Node* next;

    Node(int value) {
        data = value;
        next = NULL;
    }
};

Node* top = NULL;

void push(int value) {
    Node* newNode = new Node(value);  
    newNode->next = top;
    top = newNode;

    cout << value << " pushed into stack\n";
}

void pop() {
    if (top == NULL) {
        cout << "Stack Underflow\n";
        return;
    }

    Node* temp = top;
    cout << temp->data << " popped from stack\n";
    top = top->next;
    delete temp;
}

void peek() {
    if (top == NULL) {
        cout << "Stack is Empty\n";
    }
    else {
        cout << "Top Element = " << top->data << endl;
    }
}

void display() {
    if (top == NULL) {
        cout << "Stack is Empty\n";
        return;
    }

    Node* temp = top;
    cout << "Stack Elements: ";
    while (temp != NULL) {
        cout << temp->data << " ";
        temp = temp->next;
    }
    cout << endl;
}

int main() {
    push(10);
    push(20);
    push(30);
    display();
    peek();
    pop();
    display();

    return 0;
}

// Time Complexity:
// push()   = O(1)
// pop()    = O(1)
// peek()   = O(1)
// display()= O(n)

// Space Complexity:
// Overall stack requires O(n) space because one linked list node is created for each element.