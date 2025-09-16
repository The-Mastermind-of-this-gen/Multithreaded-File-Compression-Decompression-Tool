#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <bitset>

template<typename T>
class HashTable
{
private:
    struct Node
    {
        char* key;
        T value;
        Node* next;

        Node(const char* k, T v)
        {
            key = new char[strlen(k) + 1];
            strcpy(key, k);
            value = v;
            next = nullptr;
        }

        ~Node()
        {
            delete[] key;
        }
    };

    int hash(const char* k)
    {
        int hashValue = 0;
        size_t len = strlen(k);
        for (size_t i = 0; i < len; i++)
        {
            hashValue += k[i];
        }
        return hashValue % tableSize;
    }

    int tableSize;
    Node** buckets;

public:
    HashTable(int size = 10)
    {
        tableSize = size;
        buckets = new Node * [tableSize];
        for (int i = 0; i < tableSize; i++) {
            buckets[i] = nullptr;
        }
    }

    ~HashTable()
    {
        for (int i = 0; i < tableSize; i++)
        {
            Node* current = buckets[i];
            while (current != nullptr)
            {
                Node* next = current->next;
                delete current;
                current = next;
            }
        }
        delete[] buckets;
    }

    void insert(const char* k, T v)
    {
        int index = hash(k);
        Node* newNode = new Node(k, v);
        newNode->next = buckets[index];
        buckets[index] = newNode;
    }

    bool find(const char* k, T& result)
    {
        int index = hash(k);
        Node* current = buckets[index];

        while (current != nullptr)
        {
            if (strcmp(current->key, k) == 0)
            {
                result = current->value;
                return true;
            }
            current = current->next;
        }

        return false;
    }
};
