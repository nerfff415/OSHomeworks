#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUF_SIZE 256

int main(int argc, char *argv[]) {
    int client_sock, server_sock, port;
    struct sockaddr_in server_addr, client_addr;

    if (argc < 3) {
        printf("Неверный формат ввода\n");
        exit(1);
    }

    port = atoi(argv[2]);

    if ((server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("socket() failed");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_port = htons(port);

    if (bind(server_sock, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
        perror("bind() failed");
        exit(1);
    }
    if (listen(server_sock, 1) < 0) {
        perror("listen() failed");
        exit(1);
    }

    socklen_t client_len = sizeof(client_addr);

    if ((client_sock = accept(server_sock, (struct sockaddr*) &client_addr, &client_len)) < 0) {
        perror("accept() failed");
        exit(1);
    }

    char buf[BUF_SIZE];

    while (1) {
        memset(buf, 0, BUF_SIZE);

        if (recv(client_sock, buf, BUF_SIZE, 0) < 0) {
            perror("recv() failed");
            exit(1);
        }

        if (strcmp(buf, "The End") == 0) {
            printf("The end!\n");
            break;
        }

        printf("Полученное сообщение: %s\n", buf);

        if (send(client_sock, buf, strlen(buf), 0) < 0) {
            perror("send() failed");
            exit(1);
        }

        printf("Сообщение: %s\n", buf);
    }

   
    close(client_sock);
    close(server_sock);

    return 0;
}
