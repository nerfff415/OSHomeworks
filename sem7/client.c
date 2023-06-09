#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <signal.h>

#define SHARED_MEM_NAME "/my_shared_memory"
#define SHARED_MEM_SIZE sizeof(int)

int shm_fd;
int* shared_memory;

void handle_sigint(int sig) {
    // Закрытие отображенной области памяти
    if (munmap(shared_memory, SHARED_MEM_SIZE) == -1) {
        perror("munmap");
        exit(1);
    }

    exit(0);
}

int main() {
    // Открытие существующего объекта разделяемой памяти
    shm_fd = shm_open(SHARED_MEM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    // Отображение разделяемой памяти в адресное пространство процесса клиента
    shared_memory = (int*)mmap(NULL, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_memory == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    // Установка обработчика сигнала SIGINT
    signal(SIGINT, handle_sigint);

    // Генерация случайных чисел
    srand(time(NULL));
    while (1) {
        int random_number = rand() % 100; // Генерация числа в диапазоне от 0 до 99
        *shared_memory = random_number;
        sleep(1); // Имитация генерации чисел
    }

    return 0;
}
