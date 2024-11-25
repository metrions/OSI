#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <iostream>
#include <string>
#include <mutex>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")

#define PORT 2002
#define BUFFER_SIZE 1024

using namespace std;

mutex m;

struct list {
    list* next;
    list* prev;
    SOCKET sock;
    string port;
    string ip;
    string name;
};

// Функция для отправки сообщений всем клиентам
void sendMessages(list* clients, const string& message, const list& user) {
    list* point = clients;
    while (point) {
        string fullMessage = user.name + "(IP=" + user.ip + ":" + user.port + ") : " + message;
        send(point->sock, fullMessage.c_str(), fullMessage.size(), 0);
        point = point->next;
    }
}

// Функция для аутентификации клиента
list* auth(list& clients, SOCKET clientSock, const string& IP, const string& port) {
    lock_guard<mutex> lock(m);  // Защищаем доступ к списку клиентов

    // Создаем нового клиента
    list* newClient = new list;
    newClient->sock = clientSock;
    newClient->ip = IP;
    newClient->port = port;
    newClient->next = nullptr;

    // Добавляем клиента в конец списка
    list* point = &clients;
    while (point->next) {
        point = point->next;
    }
    point->next = newClient;
    newClient->prev = point;

    // Запрашиваем имя клиента
    string text = "Input your name: ";
    send(clientSock, text.c_str(), text.size(), 0);
    
    char buffer[BUFFER_SIZE] = {0};
    int bytesReceived = recv(clientSock, buffer, BUFFER_SIZE - 1, 0);  // Оставляем место для \0
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';  // Завершаем строку
        newClient->name = string(buffer);
    }

    // Сообщение о подключении клиента
    string message = "Client connected: " + newClient->name + 
                     " IP = " + newClient->ip + 
                     " Port = " + newClient->port;
    cout << message << endl;
    sendMessages(&clients, message, *newClient);

    return newClient;
}

// Функция для обработки сообщений от клиента
void connectClient(list* clients, list* user) {
    char buffer[BUFFER_SIZE];
    while (true) {
        int bytesReceived = recv(user->sock, buffer, BUFFER_SIZE - 1, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';  // Завершаем строку

            string response(buffer);
            if (response == "exit") break;

            cout << user->ip << ": " << response << endl;
            sendMessages(clients, response, *user);
        } else if (bytesReceived == 0) {
            cout << "Client disconnected: " << user->ip << endl;
            break;
        }
    }

    // Закрываем соединение с клиентом
    sendMessages(clients, "Client disconnected: " + user->name + 
                 " IP = " + user->ip, *user);

    // Удаляем клиента из списка
    lock_guard<mutex> lock(m);
    if (user->prev) user->prev->next = user->next;
    if (user->next) user->next->prev = user->prev;
    delete user;
    closesocket(user->sock);
}

int main() {
    WSADATA wsaData;
    WORD ver = MAKEWORD(2, 2);
    if (WSAStartup(ver, &wsaData) != 0) {
        cerr << "WSAStartup failed." << endl;
        return 1;
    }

    SOCKET servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (servSock == INVALID_SOCKET) {
        cerr << "Unable to create socket." << endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(PORT);
    sin.sin_addr.s_addr = INADDR_ANY;

    if (bind(servSock, (sockaddr*)&sin, sizeof(sin)) == SOCKET_ERROR) {
        cerr << "Unable to bind socket." << endl;
        closesocket(servSock);
        WSACleanup();
        return 1;
    }

    if (listen(servSock, 10) == SOCKET_ERROR) {
        cerr << "Unable to listen on socket." << endl;
        closesocket(servSock);
        WSACleanup();
        return 1;
    }

    cout << "Server is running on port: " << PORT << endl;

    // Инициализация списка клиентов
    list clients;
    clients.next = nullptr;
    clients.prev = nullptr;
    clients.sock = INVALID_SOCKET;

    while (true) {
        sockaddr_in clientAddr;
        int clientAddrSize = sizeof(clientAddr);
        SOCKET clientSock = accept(servSock, (sockaddr*)&clientAddr, &clientAddrSize);

        if (clientSock != INVALID_SOCKET) {
            string clientIP = inet_ntoa(clientAddr.sin_addr);
            string port = to_string(ntohs(clientAddr.sin_port));

            // Аутентифицируем клиента и создаем для него поток
            list* newClient = auth(clients, clientSock, clientIP, port);
            thread tr(connectClient, &clients, newClient);
            tr.detach();
        } else {
            cerr << "Failed to accept client connection." << endl;
        }
    }

    closesocket(servSock);
    WSACleanup();
    return 0;
}
