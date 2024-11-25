# Используем официальный образ Ubuntu
FROM ubuntu:latest

# Устанавливаем необходимые пакеты
RUN apt-get update && apt-get install -y \
    g++ \
    make \
    cmake \
    libpthread-stubs0-dev \
    net-tools

# Устанавливаем рабочую директорию
WORKDIR /app

# Копируем исходный код в контейнер
COPY . .

# Компилируем программу
RUN g++ -std=c++11 -pthread -o server serv.cpp

# Указываем порт для приложения
EXPOSE 2002

# Запускаем сервер
CMD ["./server"]
