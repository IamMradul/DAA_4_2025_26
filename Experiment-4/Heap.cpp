// IamMradul
#include <bits/stdc++.h>
using namespace std;

#define MAX 100

int heap[MAX];
int heapSize = 0;

void insert(int value) {
    if (heapSize == MAX) {
        cout << "Heap Overflow\n";
        return;
    }

    int i = heapSize;
    heap[i] = value;
    heapSize++;

    while (i != 0 && heap[(i - 1) / 2] < heap[i]) {
        swap(heap[i], heap[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
}

void heapify(int i) {
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (left < heapSize && heap[left] > heap[largest])
        largest = left;

    if (right < heapSize && heap[right] > heap[largest])
        largest = right;

    if (largest != i) {
        swap(heap[i], heap[largest]);
        heapify(largest);
    }
}

int deleteRoot() {
    if (heapSize <= 0) {
        cout << "Heap Underflow\n";
        return -1;
    }

    int root = heap[0];
    heap[0] = heap[heapSize - 1];
    heapSize--;

    heapify(0);

    return root;
}

void printHeap() {
    for (int i = 0; i < heapSize; i++)
        cout << heap[i] << " ";
    cout << endl;
}

int main() {

    insert(10);
    insert(30);
    insert(20);
    insert(5);
    insert(40);

    cout << "Heap after insertion:\n";
    printHeap();

    cout << "Deleted root: " << deleteRoot() << endl;

    cout << "Heap after deletion:\n";
    printHeap();

    return 0;
}