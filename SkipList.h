#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>

struct Node
{
    std::string key;
    std::string value;

    std::vector<Node *> forward;

    Node(std::string k, std::string v, int level)
        : key(k), value(v), forward(level + 1, nullptr) {}
};

class SkipList
{
private:
    int maxLevel;
    float probability;
    int currentLevel;
    Node *header;

public:
    SkipList(int maxLvl = 6, float p = 0.5)
        : maxLevel(maxLvl), probability(p), currentLevel(0)
    {

        header = new Node("", "", maxLevel);
        srand(time(0));
    }

    int randomLevel()
    {
        int lvl = 0;
        while ((float)rand() / RAND_MAX < probability && lvl < maxLevel)
        {
            lvl++;
        }
        return lvl;
    }

    void insert(const std::string &key, const std::string &value)
    {
        std::vector<Node *> update(maxLevel + 1);
        Node *current = header;

        for (int i = currentLevel; i >= 0; i--)
        {
            while (current->forward[i] != nullptr &&
                   current->forward[i]->key < key)
            {
                current = current->forward[i];
            }
            update[i] = current;
        }

        int rLevel = randomLevel();
        if (rLevel > currentLevel)
        {
            for (int i = currentLevel + 1; i <= rLevel; i++)
            {
                update[i] = header;
            }
            currentLevel = rLevel;
        }

        Node *newNode = new Node(key, value, rLevel);

        for (int i = 0; i <= rLevel; i++)
        {
            newNode->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = newNode;
        }
    }

    std::string search(const std::string &key)
    {
        Node *current = header;

        for (int i = currentLevel; i >= 0; i--)
        {
            while (current->forward[i] != nullptr &&
                   current->forward[i]->key < key)
            {
                current = current->forward[i];
            }
        }

        current = current->forward[0];

        if (current != nullptr && current->key == key)
        {
            return current->value;
        }
        return "";
    }

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

    void clear()
    {
        for (int i = 0; i <= maxLevel; i++)
            header->forward[i] = nullptr;
        currentLevel = 0;
    }

    bool isEmpty()
    {
        return header->forward[0] == nullptr;
    }
};