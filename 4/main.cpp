#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<WinSock.h>
#pragma comment(lib, "Ws2_32.lib")

void MAC_print(FILE *out, unsigned char *MAC);
void IP_print(FILE *out, unsigned char *IP);

int main() {
    FILE *in = NULL;
    FILE *out = NULL;
    bool enterfile = false;
    char filename[100];
    int filesize = 0;
    int framenumber = 1;
    int frame_ip = 0, frame_arp = 0, frame_snap = 0, frame_8023 = 0;
    int dix = 0;
    unsigned char *DATA;

    // Ввод имени файла
    while(!enterfile) {
        printf("File name: ");
        fgets(filename, sizeof(filename), stdin);
        filename[strcspn(filename, "\n")] = 0;  // Убираем символ новой строки
        in = fopen(filename, "rb");
        if(in != NULL) enterfile = true;
        else printf("This file does not exist! Try again.\n\n");
    }

    // Открытие файла для вывода
    out = fopen("out.txt", "w");

    // Чтение данных из файла
    fseek(in, 0, SEEK_END);
    filesize = ftell(in);
    fseek(in, 0, SEEK_SET);
    DATA = (unsigned char*)malloc(filesize);
    fread(DATA, filesize, 1, in);
    fclose(in);

    fprintf(out, "Size of file: %d bytes\n", filesize);
    fprintf(out, "\n----------------\n");

    unsigned char *p = DATA;

    while(p < DATA + filesize) {
        fprintf(out, "Frame number: %d\n", framenumber);
        fprintf(out, "MAC dest: ");
        MAC_print(out, p);  // MAC адрес назначения
        fprintf(out, "MAC source: ");
        MAC_print(out, p + 6);  // MAC адрес источника
        
        // Проверка поля длины/типа (13-й и 14-й байты)
        unsigned short LT = ntohs(*(unsigned short*)(p + 12));  // Правильное чтение 16-битного значения
        
        // Если значение поля типа протокола больше 0x05DC, это Ethernet II
        if (LT == 0x0800) {  // IPv4
            fprintf(out, "Frame type: IPv4\n");
            fprintf(out, "IP source: ");
            IP_print(out, p + 26);  // IP адрес источника
            fprintf(out, "IP dest: ");
            IP_print(out, p + 30);  // IP адрес назначения
            frame_ip++;
            p += ntohs(*(unsigned short*)(p + 16)) + 14;  // Длина кадра IPv4 + заголовок
        } else if (LT == 0x0806) {  // ARP
            fprintf(out, "Frame type: ARP\n");
            frame_arp++;
            p += 42;  // Длина ARP кадра
        } else if (LT >= 0x8137 && LT <= 0x8138) {  // Novell IPX
            fprintf(out, "Frame type: Novell IPX\n");
            p += 30;
        } else if (LT == 0x8100) {  // Ethernet VLAN
            fprintf(out, "Frame type: Ethernet VLAN\n");
            p += 18;  // Длина VLAN кадра
        } else if (LT > 0x05FE) {  // Если значение больше порога, это Ethernet II
            fprintf(out, "Frame type: Ethernet DIX (Ethernet II)\n");
            dix++;
        } else {  
            // Этап 3: Если значение меньше или равно 0x05DC, продолжаем проверку
            unsigned short F = ntohs(*(unsigned short*)(p + 14));
            
            // Этап 4: Если первые два байта равны 0xFFFF, это Ethernet_802.3 для NetWare 3.x
            if (F == 0xFFFF) {
                fprintf(out, "Frame type: Raw 802.3 (Ethernet 802.3)\n");
                frame_8023++;
            }
            // Этап 5: Если первые два байта равны 0xAAAA, это Ethernet_SNAP
            else if (F == 0xAAAA) {
                fprintf(out, "Frame type: Ethernet SNAP\n");
                frame_snap++;
            }
            // Этап 6: Иначе, это стандартный Ethernet 802.2 (LLC)
            else {
                fprintf(out, "Frame type: 802.3/LLC (Ethernet 802.2)\n");
                frame_8023++;
            }
        }

        fprintf(out, "\n---------\n");
        p += 60;  // Пропускаем стандартный кадр (если кадры разной длины, этот участок должен быть изменен)
        framenumber++;
    }

    fprintf(out, "Total number of frames: %d\n", framenumber - 1);
    fprintf(out, "IPv4 frames: %d\n", frame_ip);
    fprintf(out, "Dix frames: %d\n", dix);
    fprintf(out, "ARP frames: %d\n", frame_arp);
    fprintf(out, "SNAP frames: %d\n", frame_snap);
    fprintf(out, "802.3 frames: %d\n", frame_8023);

    fclose(out);
    free(DATA);
}

void MAC_print(FILE *out, unsigned char *MAC) {
    for (int i = 0; i < 6; i++) {
        fprintf(out, "%02X", MAC[i]);
        if (i < 5) fprintf(out, ":");
    }
    fprintf(out, "\n");
}

void IP_print(FILE *out, unsigned char *IP) {
    for (int i = 0; i < 4; i++) {
        fprintf(out, "%d", IP[i]);
        if (i < 3) fprintf(out, ".");
    }
    fprintf(out, "\n");
}
