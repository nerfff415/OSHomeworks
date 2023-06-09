#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <pipe1> <pipe2>\n", argv[0]);
        exit(1);
    }
    const char *pipe1 = argv[1];
    const char *pipe2 = argv[2];
    int fd1, fd2;
    fd1 = open(pipe1, O_RDONLY);
    fd2 = open(pipe2, O_WRONLY);
    char buffer[1024];
    while (read(fd1, buffer, sizeof(buffer)) > 0) {
        printf("Получено сообщение: %s\n", buffer);
        write(fd2, "Сообщение получено", strlen("Сообщение получено") + 1);
    }
    close(fd1);
    close(fd2);
    return 0;
}
