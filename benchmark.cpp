#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024
#define REQUEST_COUNT 10000

std::string randomString(int length)
{
    std::string str = "";
    static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    for (int i = 0; i < length; ++i)
    {
        str += alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    return str;
}

int main()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "WSAStartup failed.\n";
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        std::cerr << "Socket creation failed.\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr);

    if (connect(sock, (SOCKADDR *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        std::cerr << "Connection failed. Is server running?\n";
        return 1;
    }

    std::cout << "--- STARTING C++ BENCHMARK (" << REQUEST_COUNT << " requests) ---\n";

    char buffer[BUFFER_SIZE];
    srand(time(0));

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < REQUEST_COUNT; i++)
    {
        std::string key = "key_" + std::to_string(i);
        std::string value = randomString(8);

        std::string command = "SET " + key + " " + value;

        send(sock, command.c_str(), command.length(), 0);

        recv(sock, buffer, BUFFER_SIZE, 0);
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;

    std::cout << "\n--- RESULTS ---\n";
    std::cout << "Time Taken: " << diff.count() << " seconds\n";
    std::cout << "Throughput: " << (REQUEST_COUNT / diff.count()) << " requests/second\n";

    closesocket(sock);
    WSACleanup();
    return 0;
}