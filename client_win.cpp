#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib") // Link Windows Socket Lib

#define SERVER_IP "127.0.0.1" // "Localhost" (Your own computer)
#define PORT 8080
#define BUFFER_SIZE 1024

int main()
{
    // 1. Start Windows Sockets
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "WSAStartup failed.\n";
        return 1;
    }

    // 2. Create Socket
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        std::cerr << "Socket creation failed.\n";
        WSACleanup();
        return 1;
    }

    // 3. Connect to Server
    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    // Convert "127.0.0.1" to binary address
    inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr);

    std::cout << "Connecting to server...\n";
    if (connect(sock, (SOCKADDR *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        std::cerr << "Connection failed. Is the server running?\n";
        return 1;
    }

    std::cout << "--- Connected to DB Server! ---\n";
    std::cout << "Type 'EXIT' to quit.\n";

    char buffer[BUFFER_SIZE];
    std::string input;

    // 4. Chat Loop
    while (true)
    {
        std::cout << "MyDB> ";         // Prompt
        std::getline(std::cin, input); // Read whole line

        if (input == "EXIT")
            break;
        if (input.empty())
            continue;

        // Send message
        send(sock, input.c_str(), input.length(), 0);

        // Clear buffer and wait for reply
        memset(buffer, 0, BUFFER_SIZE);
        int valread = recv(sock, buffer, BUFFER_SIZE, 0);

        if (valread > 0)
        {
            std::cout << "Server: " << buffer;
        }
        else
        {
            std::cout << "Server disconnected.\n";
            break;
        }
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}