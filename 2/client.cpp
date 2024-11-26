#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <iostream>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

#define PORT 2002
#define BUFFER_SIZE 1024

using namespace std;

int main() {
    WSADATA wsaData;
    WORD ver = MAKEWORD(2, 2);
    if (WSAStartup(ver, &wsaData) != 0) {
        cout << "WSAStartup failed." << endl;
        return 1;
    }

    // Сокет клиента
    SOCKET clientSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSock == INVALID_SOCKET) {
        cout << "Unable to create socket." << endl;
        WSACleanup();
        return 1;
    }

    // Назначение внешнего адреса
    sockaddr_in serverInfo;
    serverInfo.sin_family = AF_INET;
    serverInfo.sin_port = htons(PORT);
    cout << "Input ip server = ";
    string ip;
    getline(cin, ip);
    char *ipChar = new char[ip.size()];
    strcpy(ipChar, ip.c_str());
    serverInfo.sin_addr.S_un.S_addr = inet_addr(ipChar); // IP-адрес сервера (localhost)

    // Установление соединения с сервером
    if (connect(clientSock, (sockaddr*)&serverInfo, sizeof(serverInfo)) == SOCKET_ERROR) {
        cout << "Unable to connect to server." << endl;
        closesocket(clientSock);
        WSACleanup();
        return 1;
    }

    cout << "Connected to server." << endl;

    while (true) {
        cout << "Enter text: ";
        string input;
        getline(cin, input);
        // Отправка текста на сервер
        send(clientSock, input.c_str(), input.size(), 0);

        char buffer[BUFFER_SIZE];
        int bytesReceived = recv(clientSock, buffer, BUFFER_SIZE, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            cout << "Received reversed string: " << buffer << endl;
            break;
        }
        closesocket(clientSock);

    }

    closesocket(clientSock);
    WSACleanup();
    return 0;
}
