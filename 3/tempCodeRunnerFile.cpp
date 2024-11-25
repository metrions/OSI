#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <iostream>
#include <string>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")

#define PORT 2002
#define BUFFER_SIZE 1024

using namespace std;

void getMessage(SOCKET clientSock) {
    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);  // Обнуляем буфер перед использованием
        int bytesReceived = recv(clientSock, buffer, BUFFER_SIZE - 1, 0);  // Оставляем место для \0
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';  // Завершаем строку
            cout << buffer << endl;
        } else if (bytesReceived == 0) {
            cout << "Server disconnected." << endl;
            break;
        } else {
            cout << "Error receiving data." << endl;
            break;
        }
    }
}

void sendMessage(SOCKET clientSock) {
    string input;
    while (true) {
        getline(cin, input);
        send(clientSock, input.c_str(), input.size(), 0);
        if (input == "exit") {
            closesocket(clientSock);
            break;
        }
    }
}

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
    string IP;
    cout << "Input ip" << endl;
    cin >> IP;
    
    char *ip = new char[IP.size() + 1];
    strcpy(ip, IP.c_str());
    serverInfo.sin_addr.S_un.S_addr = inet_addr(ip); // IP-адрес сервера (localhost)

    // Установление соединения с сервером
    if (connect(clientSock, (sockaddr*)&serverInfo, sizeof(serverInfo)) == SOCKET_ERROR) {
        cout << "Unable to connect to server." << endl;
        closesocket(clientSock);
        WSACleanup();
        return 1;
    }

    cout << "Connected to server." << endl;
    cout << "Disconect - exit" << endl;

    // Запускаем поток для получения сообщений
    thread receiverThread(getMessage, clientSock);
    
    // Отправляем сообщения в основном потоке
    sendMessage(clientSock);

    // Ожидаем завершения потока получения сообщений
    if (receiverThread.joinable()) {
        receiverThread.join();
    }

    closesocket(clientSock);
    WSACleanup();
    return 0;
}
