#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib> // for rand()
#include <ctime>   // for time()

// A Node in the Skip List
struct Node
{
    std::string key;
    std::string value;

    // Pointers to next nodes at different levels
    // forward[0] is the bottom level (next node)
    // forward[1] is the next level up (express lane)
    std::vector<Node *> forward;

    Node(std::string k, std::string v, int level)
        : key(k), value(v), forward(level + 1, nullptr) {}
};

class SkipList
{
private:
    int maxLevel;      // Maximum height of the tower (e.g., 6)
    float probability; // Probability of adding a level (Coin flip: 0.5)
    int currentLevel;  // Current highest level in the list
    Node *header;      // The starting dummy node

public:
    SkipList(int maxLvl = 6, float p = 0.5)
        : maxLevel(maxLvl), probability(p), currentLevel(0)
    {

        // Header is a dummy node that points to the start of lists
        header = new Node("", "", maxLevel);
        srand(time(0)); // Seed the random number generator
    }

    // --- Coin Flip: Decide how high a new node goes ---
    int randomLevel()
    {
        int lvl = 0;
        // Flip a coin (rand < 0.5). If heads, build up!
        while ((float)rand() / RAND_MAX < probability && lvl < maxLevel)
        {
            lvl++;
        }
        return lvl;
    }

    // --- INSERT (Put) ---
    void insert(const std::string &key, const std::string &value)
    {
        std::vector<Node *> update(maxLevel + 1);
        Node *current = header;

        // 1. Find the spot (Search from Top -> Bottom)
        for (int i = currentLevel; i >= 0; i--)
        {
            while (current->forward[i] != nullptr &&
                   current->forward[i]->key < key)
            {
                current = current->forward[i];
            }
            update[i] = current; // Remember the path
        }

        // 2. Create the new node
        int rLevel = randomLevel();
        if (rLevel > currentLevel)
        {
            // If new level is higher than current max, link header to it
            for (int i = currentLevel + 1; i <= rLevel; i++)
            {
                update[i] = header;
            }
            currentLevel = rLevel;
        }

        Node *newNode = new Node(key, value, rLevel);

        // 3. Update pointers (The "Rewiring")
        for (int i = 0; i <= rLevel; i++)
        {
            newNode->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = newNode;
        }
    }

    // --- SEARCH (Get) ---
    std::string search(const std::string &key)
    {
        Node *current = header;

        // Start from top, go right, then down
        for (int i = currentLevel; i >= 0; i--)
        {
            while (current->forward[i] != nullptr &&
                   current->forward[i]->key < key)
            {
                current = current->forward[i];
            }
        }

        // We are now at the node just before the target (at bottom level)
        current = current->forward[0];

        if (current != nullptr && current->key == key)
        {
            return current->value;
        }
        return ""; // Not found
    }

    // --- Helper for Flush: Get ALL data ---
    // Returns a simple vector of pairs so KVStore can write to disk
    std::vector<std::pair<std::string, std::string>> getAll()
    {
        std::vector<std::pair<std::string, std::string>> result;
        Node *current = header->forward[0]; // Level 0 has ALL nodes

        while (current != nullptr)
        {
            result.push_back({current->key, current->value});
            current = current->forward[0];
        }
        return result;
    }

    // --- Helper: Clear RAM ---
    void clear()
    {
        // (Simple version: reset pointers. Real version needs 'delete')
        for (int i = 0; i <= maxLevel; i++)
            header->forward[i] = nullptr;
        currentLevel = 0;
    }

    // --- Helper: Check Size ---
    // (Approximate size check for flushing trigger)
    bool isEmpty()
    {
        return header->forward[0] == nullptr;
    }
};