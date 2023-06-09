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
    mkfifo(pipe1, 0666);
    mkfifo(pipe2, 0666);
    fd1 = open(pipe1, O_WRONLY);
    fd2 = open(pipe2, O_RDONLY);
    char *messages[] = {"Привет!", "Как дела?", "Пока!"};
    int num_messages = sizeof(messages) / sizeof(messages[0]);

    for (int i = 0; i < num_messages; i++) {
        write(fd1, messages[i], strlen(messages[i]) + 1);
        char response[1024];
        read(fd2, response, sizeof(response));
        printf("Получен ответ: %s\n", response);
        sleep(10);
    }
    close(fd1);
    close(fd2);
    unlink(pipe1);
    unlink(pipe2);
    return 0;
}
