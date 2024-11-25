#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <iostream>
#include <string>
#include <algorithm>

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
    // Сокет сервера
    SOCKET servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    // Если сокет занят
    if (servSock == INVALID_SOCKET) {
        cout << "Unable to create socket." << endl;
        WSACleanup();
        return 1;
    }

    // Назначение внешнего адреса
    sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(PORT);
    sin.sin_addr.s_addr = INADDR_ANY;

    if (bind(servSock, (sockaddr*)&sin, sizeof(sin)) == SOCKET_ERROR) {
        cout << "Unable to bind socket." << endl;
        closesocket(servSock);
        WSACleanup();
        return 1;
    }

    // Режим прослушивания с макс буфером - 10
    if (listen(servSock, 10) == SOCKET_ERROR) {
        cout << "Unable to listen on socket." << endl;
        closesocket(servSock);
        WSACleanup();
        return 1;
    }

    cout << "Server started on port " << PORT << endl;

    while (true) {
        sockaddr_in clientAddr;
        int clientAddrSize = sizeof(clientAddr);
        // Сокет клиента (Ожидаем запрос на установление связи)
        SOCKET clientSock = accept(servSock, (sockaddr*)&clientAddr, &clientAddrSize);

        if (clientSock == INVALID_SOCKET) {
            cout << "Failed to accept connection." << endl;
            continue;
        }
        char* clientIp= inet_ntoa(clientAddr.sin_addr);
        u_short port= clientAddr.sin_port;
        cout << "Client connected with ip: "<<clientIp<<":"<< port << endl;

        char buffer[BUFFER_SIZE];
        // Получаем данные от клиента -> buffer
        int bytesReceived = recv(clientSock, buffer, BUFFER_SIZE, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            cout << "Received: " << buffer << endl;

            string response(buffer);
            // Реверс строки
            reverse(response.begin(), response.end());

            // Отправка клиенту
            send(clientSock, response.c_str(), response.size(), 0);
            cout << "Sent reversed string: " << response << endl;
        }
        
        // Закрываем соединение с клиентом
        closesocket(clientSock);
    }

    closesocket(servSock);
    WSACleanup();
    return 0;
}