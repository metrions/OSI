# Определяем протоколы
PROT_IPV4 = 0x0800
PROT_ARP = 0x0806
PROT_IPV6 = 0x08DD
PROT_IPX = 0x8137

input_file = input("Введите название файла для чтения: ")
try:
    with open(input_file, "rb") as f:
        buffer = bytearray(f.read()) # Читаем весь файл в буфер
        print(f"Файл \"{input_file}\" успешно считан.")
except FileNotFoundError:
    print(f"Файл \"{input_file}\" отсутствует.")
    exit(0)

count_of_frames = 0  # Счетчик фреймов
count_of_frames_type = [0] * 4  # Кол-во обработанных типов фреймов
count_of_protocol = [0] * 3  # Кол-во обработанных типов протоколов

print(f"\n{'-' * 30}\n")
current = 0  # Текущий элемент
total_size = 0  # Общий размер

while current < len(buffer):
    count_of_frames += 1
    print(f"Фрейм {count_of_frames}")

    print("MAC-адрес получателя: ")
    print(':'.join(f"{buffer[current + i]:02X}" for i in range(6)))

    print("MAC-адрес отправителя: ")
    print(':'.join(f"{buffer[current + 6 + i]:02X}" for i in range(6)))

    # Чтение типа протокола или длины
    len_type = int(buffer[current + 12:current + 14].hex(), 16)
    if len_type > 0x05DC:
        count_of_frames_type[0] += 1
        print("Тип фрейма: Ethernet II")

        if len_type == PROT_IPV4:
            count_of_protocol[0] += 1
            version = (buffer[current + 14] & 0xF0) >> 4
            head_size = (buffer[current + 14] & 0x0F) * 4
            body_size = int(buffer[current + 16:current + 18].hex(), 16)

            print(f"Версия: {version}")
            print("IP-адрес источника: ")
            print('.'.join(str(buffer[current + 26 + i]) for i in range(4)))

            print("IP-адрес назначения: ")
            print('.'.join(str(buffer[current + 30 + i]) for i in range(4)))

            len_type = body_size + 14  # Учитываем заголовок
            print(f"Размер фрейма: {len_type} байт")

            protocol = buffer[current + 23] # определяем протокол
            if protocol == 6:
                print("Протокол: TCP")
            elif protocol == 17:
                print("Протокол: UDP")

            print("Данные: ")
            for i in range(body_size - head_size):
                print(f"{buffer[current + 14 + head_size + i]:02X} ", end='')
            print()
            current += len_type
            total_size += len_type

        elif len_type == PROT_ARP:
            count_of_protocol[1] += 1
            print("Тип фрейма: ARP")

            print("MAC-адрес отправителя: ")
            print(':'.join(f"{buffer[current + 22 + i]:02X}" for i in range(6)))

            print("IP-адрес отправителя: ")
            print('.'.join(str(buffer[current + 28 + i]) for i in range(4)))

            print("MAC-адрес получателя: ")
            print(':'.join(f"{buffer[current + 32 + i]:02X}" for i in range(6)))

            print("IP-адрес получателя: ")
            print('.'.join(str(buffer[current + 38 + i]) for i in range(4)))

            print("Размер фрейма: 42 байт")
            current += 42
            total_size += 42

        elif len_type == PROT_IPV6:
            print("Тип протокола: IPv6")
        elif len_type == PROT_IPX:
            print("Тип протокола: IPX (Novell)")
        else:
            print("Тип протокола: Неизвестен.")
            print(f"Номер протокола: {len_type:x}")
            current += len_type + 14
            total_size += len_type + 14
            count_of_protocol[2] += 1
    else:
        count_of_protocol[2] += 1
        llc = (buffer[current + 14] << 8) + buffer[current + 15] # тип протокола LLC
        if llc == 0xFFFF:
            count_of_frames_type[1] += 1
            print("Тип фрейма: Ethernet_802.3 для NetWare 3.х")
            print(f"Размер фрейма: {len_type + 14} байт")
            print("Данные: ")
            for i in range(len_type):
                print(f"{buffer[current + 14 + i]:02X} ", end='')
            print()
        elif llc == 0xAAAA:
            count_of_frames_type[2] += 1
            print("Тип фрейма: Ethernet_SNAP")
            print(f"Размер фрейма: {len_type + 14} байт")
            print("Данные: ")
            for i in range(len_type - 3 - 5):
                print(f"{buffer[current + 14 + 3 + 5 + i]:02X} ", end='')
            print()
        else:
            count_of_frames_type[3] += 1
            print("Тип фрейма: Ethernet_802.2")
            print(f"Размер фрейма: {len_type + 14} байт")
            print("Данные: ")
            for i in range(len_type - 3):
                print(f"{buffer[current + 14 + 3 + i]:02X} ", end='')
            print()
        current += len_type + 14
        total_size += len_type + 14

    print(f"\n{'-' * 30}\n")

#Результаты
print(f"Считано: {count_of_frames} фреймов")
print("Типы фреймов:")
print(f"Ethernet II: {count_of_frames_type[0]}")
print(f"Ethernet_802.3 для NetWare 3.х: {count_of_frames_type[1]}")
print(f"Ethernet_SNAP: {count_of_frames_type[2]}")
print(f"Ethernet_802.2: {count_of_frames_type[3]}")
print("Типы протоколов:")
print(f"IPv4: {count_of_protocol[0]}")
print(f"ARP: {count_of_protocol[1]}")
print(f"Неопределен: {count_of_protocol[2]}")
