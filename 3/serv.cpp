#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <iostream>
#include <string>
#include <mutex>
#include <algorithm>
#include <thread>

#define PORT 2002
#define BUFFER_SIZE 1024

using namespace std;

mutex m;

struct list {
    list* next;
    list* prev;
    int sock;
    string port;
    string ip;
    string name;
};

void sendMessages(list* clients, const string& message, const list& user) {
    if (clients){
        list* point = clients;
        while (point) {
            string fullMessage = user.name + "(IP=" + user.ip + ":" + user.port + ") " + ": " + message; 
            send(point->sock, fullMessage.c_str(), fullMessage.size(), 0);
            point = point->next;
        }
    }
}

list* auth(list& clients, int clientSock, string IP, string port) {
    // Добавляем клиента в список
    m.lock();
    list* point = &clients;
    while (point->next) {
        point = point->next;
    }
    list* newClient = new list;  // динамическое выделение памяти
    newClient->sock = clientSock;
    newClient->next = nullptr;
    newClient->prev = point;
    newClient->ip = IP;
    newClient->port = port;
    point->next = newClient;
    m.unlock();

    string text = "input your name";
    send(newClient->sock, text.c_str(), text.size(), 0);
    
    char buffer[BUFFER_SIZE];
    recv(newClient->sock, buffer, BUFFER_SIZE, 0);
    string response(buffer);
    newClient->name = response;
    cout << "Client connected IP=" + newClient->ip + " name=" + newClient->name << endl;
    sendMessages(&clients, "Client connected IP=" + newClient->ip + " name=" + newClient->name, *newClient);
    return newClient;  // возвращаем указатель на нового клиента
}

void connectClient(list* clients, list* user) {
    char buffer[BUFFER_SIZE];
    while (true) {
        int bytesReceived = recv(user->sock, buffer, BUFFER_SIZE, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';  // Завершаем строку
            cout << user->name + " IP(" + user->ip << "): " << buffer << endl;

            string response(buffer);
            if (response == "exit") break;
            sendMessages(clients, response, *user);
        }
    }

    // Закрываем соединение с клиентом
    cout << "Client disconnected IP=" + user->ip + " name=" + user->name << endl;
    sendMessages(clients, "Client disconnected IP=" + user->ip + " name=" + user->name, *user);

    m.lock();
    if (user->prev) user->prev->next = user->next;
    if (user->next) user->next->prev = user->prev;
    close(user->sock);
    delete user;  // освобождаем память после закрытия сокета
    m.unlock();
}

int main() {
    int servSock = socket(AF_INET, SOCK_STREAM, 0);
    if (servSock < 0) {
        cout << "Unable to create socket." << endl;
        return 1;
    }

    sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(PORT);
    sin.sin_addr.s_addr = INADDR_ANY;  // Привязываем ко всем интерфейсам

    if (bind(servSock, (sockaddr*)&sin, sizeof(sin)) < 0) {
        cout << "Unable to bind socket." << endl;
        close(servSock);
        return 1;
    }

    if (listen(servSock, 10) < 0) {
        cout << "Unable to listen on socket." << endl;
        close(servSock);
        return 1;
    }

    // Получение имени хоста
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) < 0) {
        cout << "Error getting hostname." << endl;
        close(servSock);
        return 1;
    }

    // Получение IP-адреса по имени хоста
    hostent* host = gethostbyname(hostname);
    if (host == nullptr) {
        cout << "Error getting host by name." << endl;
        close(servSock);
        return 1;
    }

    // Преобразуем в строку и выводим первый найденный IP-адрес
    string ip = inet_ntoa(*(in_addr*)host->h_addr_list[0]);
    cout << "Server is running on IP: " << ip << " and port: " << ntohs(sin.sin_port) << endl;

    // Инициализация списка клиентов
    list clients;
    clients.next = nullptr;
    clients.prev = nullptr;
    clients.sock = -1;  // Псевдоклиент в начале списка

    while (true) {
        sockaddr_in clientAddr;
        socklen_t clientAddrSize = sizeof(clientAddr);
        int clientSock = accept(servSock, (sockaddr*)&clientAddr, &clientAddrSize);

        if (clientSock >= 0) {
            string clientIP = inet_ntoa(clientAddr.sin_addr);  // IP клиента
            string port = to_string(ntohs(clientAddr.sin_port));  // Порт клиента
            list* newClient = auth(clients, clientSock, clientIP, port);  // возвращаем указатель
            thread tr(connectClient, &clients, newClient);  // передаем указатель на клиента
            tr.detach();
        } else {
            cout << "Failed to accept client connection." << endl;
        }
    }

    close(servSock);
    return 0;
}
