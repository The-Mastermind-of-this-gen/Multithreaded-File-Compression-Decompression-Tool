#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <new>

#pragma once

using namespace std;

// ===== Error Codes Enum =====
enum ReturnCodes {
    Successful_Operation = 0,
    Unlogical_Input = -1,
    HEAP_NOT_INITIALIZED = -2,
    HEAP_OVERFLOW = -3,
    HEAP_Resize_Failed = -4,
    HEAP_UNDERFLOW = -5
};

// ===== Min Heap Template =====
template<typename Input>
class Heap {
    Input* arr; // Heap array
    int Array_Capacity; // Array Array_Capacity
    int Heap_Size; // Number of elements in the heap

    void HeapifyUp(int index) {
        int parentIndex = (index - 1) / 2;
        while (index > 0 && arr[index] < arr[parentIndex]) {
            Input temp = arr[index];
            arr[index] = arr[parentIndex];
            arr[parentIndex] = temp;
            index = parentIndex;
            parentIndex = (index - 1) / 2; // update parent index
        }
    }

    void HeapifyDown() {
        int parentIndex = 0;
        while (true) {
            int leftChildIndex = 2 * parentIndex + 1;
            int rightChildIndex = 2 * parentIndex + 2;
            int SmallestChildIndex = parentIndex;

            if (leftChildIndex >= Heap_Size)
                break;
            if (rightChildIndex < Heap_Size && arr[rightChildIndex] < arr[leftChildIndex])
                SmallestChildIndex = rightChildIndex;
            else
                SmallestChildIndex = leftChildIndex;
            if (SmallestChildIndex == parentIndex)
                break;
            if (arr[parentIndex] < arr[SmallestChildIndex])
                break;
            Input temp = arr[SmallestChildIndex];
            arr[SmallestChildIndex] = arr[parentIndex];
            arr[parentIndex] = temp;
            parentIndex = SmallestChildIndex;
        }
    }

    bool isFull() {
        return Heap_Size == Array_Capacity;
    }

    bool isEmpty() {
        return Heap_Size == 0;
    }

    int Array_Capacity_Resize() {
        Input* New_Array = new(nothrow) Input[Array_Capacity + 16]; // dy 7eta mohamed hazem 16 da ana a5tarto 3shan 16^2 = 256 :)
        if (New_Array == NULL)
            return HEAP_Resize_Failed;
        for (int i = 0; i < Heap_Size; i++) {
            New_Array[i] = arr[i];
        }
        delete[] arr;
        arr = New_Array;
        Array_Capacity += 16;
        return Successful_Operation;
    }

public:
    Heap() {
        arr = NULL;
        Heap_Size = 0;
        Array_Capacity = 0;
    }

    ~Heap() {
        delete[] arr;
    }

    int Array_Size() {
        return Heap_Size;
    }

    bool initialize(int Capacity) {
        if (Capacity <= 0)
            return false;
        if (arr != NULL)
            delete[] arr;
        arr = new(nothrow) Input[Capacity];
        if (arr == NULL)
            return false;
        Array_Capacity = Capacity;
        return true;
    }

    int insert(Input Item) {
        if (arr == NULL)
            return HEAP_NOT_INITIALIZED;
        if (isFull()) {
            int What;
            What = Array_Capacity_Resize();
            if (What == HEAP_Resize_Failed)
                return HEAP_Resize_Failed;
        }
        arr[Heap_Size] = Item;
        HeapifyUp(Heap_Size);
        Heap_Size++;
        return Successful_Operation;
    }

    bool DeleteValue(Input* Item = NULL) {
        if (arr == NULL)
            return false;
        if (isEmpty())
            return false;
        if (Item != NULL)
            *Item = arr[0];

        arr[0] = arr[Heap_Size - 1];
        Heap_Size--;
        HeapifyDown();
        return true;
    }
};
