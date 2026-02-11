#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <chrono>
#include <functional>
#include <cstdio>
#include "SkipList.h"

class BloomFilter
{
private:
    std::vector<bool> bitArray;
    int size;

public:
    BloomFilter(int size = 10000) : size(size)
    {
        bitArray.resize(size, false);
    }

    int getHash(const std::string &key)
    {
        std::hash<std::string> hasher;
        return hasher(key) % size;
    }

    void add(const std::string &key)
    {
        bitArray[getHash(key)] = true;
    }

    bool mightContain(const std::string &key)
    {
        return bitArray[getHash(key)];
    }
};

struct SSTableEntry
{
    std::string filename;
    BloomFilter filter;
};

class KVStore
{
private:
    SkipList memTable;

    int memTableSize = 0;

    const std::string wal_file = "wal.log";

    std::vector<SSTableEntry> sstList;

    void appendToWAL(const std::string &key, const std::string &value)
    {
        std::ofstream file(wal_file, std::ios::app);
        if (file.is_open())
        {
            file << key << "," << value << "\n";
            file.close();
        }
    }

    std::string generateSSTableName()
    {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        return "sst_" + std::to_string(timestamp) + ".csv";
    }

    std::string searchSSTable(const std::string &filename, const std::string &key)
    {
        std::ifstream file(filename);
        std::string line;
        while (std::getline(file, line))
        {
            size_t delimiterPos = line.find(',');
            if (delimiterPos != std::string::npos)
            {
                std::string fileKey = line.substr(0, delimiterPos);
                if (fileKey == key)
                {
                    return line.substr(delimiterPos + 1);
                }
            }
        }
        return "";
    }

public:
    KVStore()
    {
        std::cout << "[System] Checking WAL for crash recovery...\n";
        std::ifstream file(wal_file);
        if (!file.is_open())
            return;

        std::string line;
        int count = 0;
        while (std::getline(file, line))
        {
            size_t delimiterPos = line.find(',');
            if (delimiterPos != std::string::npos)
            {
                std::string key = line.substr(0, delimiterPos);
                std::string value = line.substr(delimiterPos + 1);

                memTable.insert(key, value);
                memTableSize++;
                count++;
            }
        }
        std::cout << "[System] Recovered " << count << " items from WAL.\n";
    }

    void put(const std::string &key, const std::string &value)
    {
        appendToWAL(key, value);

        memTable.insert(key, value);
        memTableSize++;

        if (memTableSize > 2000)
        {
            flush();
        }
    }

    std::string get(const std::string &key)
    {
        std::string val = memTable.search(key);

        if (val != "")
        {
            return val;
        }

        for (auto it = sstList.rbegin(); it != sstList.rend(); ++it)
        {
            if (!it->filter.mightContain(key))
            {
                continue;
            }
            std::string diskVal = searchSSTable(it->filename, key);
            if (!diskVal.empty())
                return diskVal;
        }

        return "Key not found";
    }

    void flush()
    {
        if (memTable.isEmpty())
            return;

        std::string filename = generateSSTableName();
        std::ofstream file(filename);
        BloomFilter filter;

        auto allData = memTable.getAll();

        for (const auto &pair : allData)
        {
            file << pair.first << "," << pair.second << "\n";
            filter.add(pair.first);
        }
        file.close();

        sstList.push_back({filename, filter});
        std::cout << "[System] Flushed to " << filename << "\n";

        memTable.clear();
        memTableSize = 0;

        std::ofstream wal(wal_file, std::ios::trunc);
        wal.close();
    }

    void compact()
    {
        if (sstList.empty())
            return;
        std::cout << "[System] Starting Compaction...\n";

        std::map<std::string, std::string> mergedData;

        for (const auto &entry : sstList)
        {
            std::ifstream file(entry.filename);
            std::string line;
            while (std::getline(file, line))
            {
                size_t delimiterPos = line.find(',');
                if (delimiterPos != std::string::npos)
                {
                    std::string key = line.substr(0, delimiterPos);
                    std::string value = line.substr(delimiterPos + 1);
                    mergedData[key] = value;
                }
            }
            file.close();
            std::remove(entry.filename.c_str());
        }

        std::string newFilename = generateSSTableName();
        std::ofstream outFile(newFilename);
        BloomFilter newFilter;

        for (const auto &pair : mergedData)
        {
            outFile << pair.first << "," << pair.second << "\n";
            newFilter.add(pair.first);
        }
        outFile.close();

        sstList.clear();
        sstList.push_back({newFilename, newFilter});
        std::cout << "[System] Compacted all files into " << newFilename << "\n";
    }
};