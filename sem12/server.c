#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 1030
#define MAX_MSG_LEN 1024

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
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

    // Привязка сокета к адресу сервера
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        exit(1);
    }

    printf("Сервер запущен. Ожидание сообщений...\n");

    // Бесконечный цикл приема и рассылки сообщений
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);

        // Прием сообщения от клиента
        ssize_t recv_len = recvfrom(sockfd, message, MAX_MSG_LEN, 0, (struct sockaddr *)&client_addr, &addr_len);
        if (recv_len == -1) {
            perror("recvfrom");
            exit(1);
        }

        message[recv_len] = '\0';
        printf("Получено сообщение от клиента: %s\n", message);

        // Рассылка сообщения всем клиентам
        if (sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&client_addr, addr_len) == -1) {
            perror("sendto");
            exit(1);
        }

        printf("Сообщение отправлено клиенту\n");
    }

    // Закрытие сокета
    close(sockfd);

    return 0;
}
