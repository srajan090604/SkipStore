#include <iostream>
#include <string>
#include <sstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "KVStore.h"

#pragma comment(lib, "Ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 1024

void handleClient(SOCKET clientSocket, KVStore &db)
{
    char buffer[BUFFER_SIZE];

    while (true)
    {

        memset(buffer, 0, BUFFER_SIZE);

        int valread = recv(clientSocket, buffer, BUFFER_SIZE, 0);

        if (valread <= 0)
        {
            std::cout << "Client hung up (Disconnected).\n";
            closesocket(clientSocket);
            break;
        }

        std::string request(buffer);
        std::stringstream ss(request);
        std::string cmd, key, value;
        ss >> cmd;

        std::string response;

        if (cmd == "SET")
        {
            ss >> key >> value;
            db.put(key, value);
            response = "OK\n";
        }
        else if (cmd == "GET")
        {
            ss >> key;
            std::string result = db.get(key);
            if (result == "Key not found")
                response = "(nil)\n";
            else
                response = result + "\n";
        }
        else if (cmd == "FLUSH")
        {
            db.flush();
            response = "Flushed to Disk\n";
        }
        else
        {
            response = "ERROR: I don't know that command\n";
        }

        send(clientSocket, response.c_str(), response.length(), 0);
    }
}

int main()
{
    KVStore db;

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "Failed to start Winsock.\n";
        return 1;
    }

    SOCKET server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_fd == INVALID_SOCKET)
    {
        std::cerr << "Could not create socket.\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (SOCKADDR *)&address, sizeof(address)) == SOCKET_ERROR)
    {
        std::cerr << "Bind failed. Is Port 8080 already used?\n";
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    if (listen(server_fd, 3) == SOCKET_ERROR)
    {
        std::cerr << "Listen failed.\n";
        return 1;
    }

    std::cout << "--- Server Started on Port " << PORT << " ---\n";
    std::cout << "Waiting for connections...\n";

    int addrlen = sizeof(address);
    while (true)
    {
        SOCKET new_socket = accept(server_fd, (SOCKADDR *)&address, &addrlen);

        if (new_socket == INVALID_SOCKET)
        {
            std::cerr << "Accept failed.\n";
            continue;
        }

        std::cout << "New Client Connected!\n";

        handleClient(new_socket, db);
    }

    closesocket(server_fd);
    WSACleanup();
    return 0;
}