#include <iostream>
#include <string>
#include <sstream>
#include <winsock2.h> // This is the Windows "Phone" library
#include <ws2tcpip.h> // Helper tools for the phone
#include "KVStore.h"  // Your Database Brain

// This tells the compiler to link the Windows Socket Library (Ws2_32.lib)
// Without this, you get weird "undefined reference" errors.
#pragma comment(lib, "Ws2_32.lib")

#define PORT 8080        // The "Phone Number" extension we listen on
#define BUFFER_SIZE 1024 // How much data we can read at once (1KB)

// --- The Conversation Handler ---
// This function runs when someone calls our server.
void handleClient(SOCKET clientSocket, KVStore &db)
{
    char buffer[BUFFER_SIZE];

    while (true)
    {
        // 1. Clear the buffer (wipe the whiteboard clean)
        memset(buffer, 0, BUFFER_SIZE);

        // 2. Read what the client said
        // recv() waits here until data arrives
        int valread = recv(clientSocket, buffer, BUFFER_SIZE, 0);

        if (valread <= 0)
        {
            std::cout << "Client hung up (Disconnected).\n";
            closesocket(clientSocket);
            break; // Exit the loop
        }

        // 3. Parse the message (e.g., "SET user 1")
        std::string request(buffer);
        std::stringstream ss(request);
        std::string cmd, key, value;
        ss >> cmd; // Read first word

        std::string response;

        // 4. Ask the Brain (KVStore) what to do
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

        // 5. Send the answer back
        send(clientSocket, response.c_str(), response.length(), 0);
    }
}

int main()
{
    // 1. Wake up the Brain
    KVStore db;

    // --- WINDOWS SPECIAL STEP ---
    // On Windows, you must "Start Up" the socket library before using it.
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "Failed to start Winsock.\n";
        return 1;
    }

    // 2. Buy a Phone (Create Socket)
    // AF_INET = Internet (IPv4)
    // SOCK_STREAM = TCP (Reliable connection)
    SOCKET server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_fd == INVALID_SOCKET)
    {
        std::cerr << "Could not create socket.\n";
        WSACleanup(); // Clean up if we fail
        return 1;
    }

    // 3. Plug the Phone into the Wall (Bind)
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Listen to anyone
    address.sin_port = htons(PORT);       // Bind to Port 8080

    if (bind(server_fd, (SOCKADDR *)&address, sizeof(address)) == SOCKET_ERROR)
    {
        std::cerr << "Bind failed. Is Port 8080 already used?\n";
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    // 4. Wait for Rings (Listen)
    if (listen(server_fd, 3) == SOCKET_ERROR)
    {
        std::cerr << "Listen failed.\n";
        return 1;
    }

    std::cout << "--- Server Started on Port " << PORT << " ---\n";
    std::cout << "Waiting for connections...\n";

    // 5. Answer the Phone (Accept Loop)
    // This loop runs forever.
    int addrlen = sizeof(address);
    while (true)
    {
        // accept() freezes the program until someone connects!
        SOCKET new_socket = accept(server_fd, (SOCKADDR *)&address, &addrlen);

        if (new_socket == INVALID_SOCKET)
        {
            std::cerr << "Accept failed.\n";
            continue;
        }

        std::cout << "New Client Connected!\n";

        // Start the conversation
        handleClient(new_socket, db);
    }

    // Cleanup (We never reach here, but it's good practice)
    closesocket(server_fd);
    WSACleanup();
    return 0;
}