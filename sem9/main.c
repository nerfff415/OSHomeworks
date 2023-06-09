#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <string.h>

#define MESSAGE_LIMIT 10

int main() {
    int pipefd[2]; // Дескрипторы для канала
    pid_t pid;
    sem_t* semaphore;

    // Создание неименованного канала
    if (pipe(pipefd) == -1) {
        perror("Ошибка при создании канала");
        exit(EXIT_FAILURE);
    }

    // Создание семафора
    semaphore = sem_open("/my_semaphore", O_CREAT | O_EXCL, 0644, 1);
    if (semaphore == SEM_FAILED) {
        perror("Ошибка при создании семафора");
        exit(EXIT_FAILURE);
    }

    // Создание дочернего процесса
    pid = fork();

    if (pid < 0) {
        perror("Ошибка при создании дочернего процесса");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Код дочернего процесса

        int i;
        char* message;

        close(pipefd[1]); // Закрываем дескриптор для записи в канал
        printf("Дочерний процесс начал принимать сообщения:\n");

        for (i = 0; i < MESSAGE_LIMIT; i++) {
            sem_wait(semaphore); // Ждем, пока родительский процесс напишет

            // Читаем размер сообщения
            size_t message_length;
            read(pipefd[0], &message_length, sizeof(size_t));

            // Выделяем память для буфера сообщения
            message = (char*) malloc((message_length + 1) * sizeof(char));

            // Читаем само сообщение
            read(pipefd[0], message, message_length);
            message[message_length] = '\0';

            printf("Дочерний процесс принял сообщение: %s\n", message);

            free(message); // Освобождаем память

            sem_post(semaphore); // Разрешаем родительскому процессу писать
        }

        close(pipefd[0]); // Закрываем дескриптор для чтения из канала
        sem_close(semaphore); // Закрываем семафор
        sem_unlink("/my_semaphore"); // Удаляем семафор

        printf("Дочерний процесс завершен\n");
        exit(EXIT_SUCCESS);

    } else {
        // Код родительского процесса

        int i;
        char* messages[] = {"Привет!", "Как дела?", "Это родительский процесс", "Все хорошо", "А у тебя?", "Давно не виделись", "У меня все отлично", "Что нового?", "До свидания!", "Пока!"};

        close(pipefd[0]); // Закрываем дескриптор для чтения из канала
        printf("Родительский процесс начал отправку сообщений:\n");

        for (i = 0; i < MESSAGE_LIMIT; i++) {
            sem_wait(semaphore); // Ждем, пока дочерний процесс примет сообщение

            size_t message_length = strlen(messages[i]);

            // Пишем размер сообщения
            write(pipefd[1], &message_length, sizeof(size_t));

            // Пишем само сообщение
            write(pipefd[1], messages[i], message_length);

            printf("Родительский процесс отправил сообщение: %s\n", messages[i]);
            sem_post(semaphore); // Разрешаем дочернему процессу принимать
        }

        close(pipefd[1]); // Закрываем дескриптор для записи в канал
        sem_close(semaphore); // Закрываем семафор
        sem_unlink("/my_semaphore"); // Удаляем семафор

        printf("Родительский процесс завершен\n");
        wait(NULL); // Ждем завершения дочернего процесса
        exit(EXIT_SUCCESS);
    }

    return 0;
}
