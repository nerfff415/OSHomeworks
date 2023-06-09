#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "192.168.0.255"  // Широковещательный адрес в вашей сети
#define PORT 1030
#define MAX_MSG_LEN 1024

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    char message[MAX_MSG_LEN];

    // Создание сокета
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    // Настройка адреса сервера
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Привязка клиента к адресу (необходимо только для приема сообщений)
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        exit(1);
    }

    // Настройка широковещательной опции
    int broadcast_enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable)) == -1) {
        perror("setsockopt");
        exit(1);
    }

    // Цикл ввода и отправки сообщений
    while (1) {
        printf("Введите сообщение для отправки (для выхода введите 'exit'): ");
        fgets(message, MAX_MSG_LEN, stdin);

        // Удаление символа новой строки из сообщения
        message[strcspn(message, "\n")] = '\0';

        // Проверка условия выхода из цикла
        if (strcmp(message, "exit") == 0)
            break;

        // Отправка сообщения на сервер
        if (sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
            perror("sendto");
            exit(1);
        }

        printf("Сообщение отправлено серверу\n");

        // Ожидание ответа от сервера
        ssize_t recv_len = recvfrom(sockfd, message, MAX_MSG_LEN, 0, NULL, NULL);
        if (recv_len == -1) {
            perror("recvfrom");
            exit(1);
        }

        message[recv_len] = '\0';
        printf("Получен ответ от сервера: %s\n", message);
    }

    // Закрытие сокета
    close(sockfd);

    return 0;
}
