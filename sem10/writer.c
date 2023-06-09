#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#define BUF_SIZE 10

typedef struct {
    int store[BUF_SIZE];
    int have_reader;
    int have_writer;
    pid_t writer_pid;
} shared_memory;

pthread_mutex_t lock_reader, lock_writer;

int main(int argc, char *argv[]) {
    char *shar_object = "/my_shared_memory";
    sem_t *mutex, *empty, *full, *admin;
    shared_memory *buffer;
    int buf_id, counter = 0, current_index = 0;

    // Получение доступа к кольцевому буферу
    if ((buf_id = shm_open(shar_object, O_RDWR, 0666)) == -1) {
        perror("shm_open");
        exit(-1);
    }

    // Получить доступ к памяти
    buffer = mmap(0, sizeof(shared_memory), PROT_WRITE | PROT_READ, MAP_SHARED, buf_id, 0);
    if (buffer == (shared_memory *) -1) {
        perror("writer: mmap");
        exit(-1);
    }

    // Разборки писателей. Мьютекс для конкуренции за работу
    pthread_mutex_init(&lock_writer, NULL);

    // Инициализация семафоров
    if ((mutex = sem_open("/mutex", 0)) == SEM_FAILED) {
        perror("sem_open: Can not open mutex semaphore");
        exit(-1);
    };
    if ((empty = sem_open("/empty", 0)) == SEM_FAILED) {
        perror("sem_open: Can not open empty semaphore");
        exit(-1);
    };
    if ((full = sem_open("/full", 0)) == SEM_FAILED) {
        perror("sem_open: Can not open full semaphore");
        exit(-1);
    };
    if ((admin = sem_open("/admin", 0)) == SEM_FAILED) {
        perror("sem_open: Can not open admin semaphore");
        exit(-1);
    };

    // Алгоритм писателя
    while (1) {
        // Блокирование семафора администратора
        if (sem_wait(admin) == -1) {
            perror("sem_wait: Can not wait admin semaphore");
            exit(-1);
        }

        // Запись в буфер
        if (sem_wait(empty) == -1) {
            perror("sem_wait: Can not wait empty semaphore");
            exit(-1);
        };

        // Захват семафора мьютекса для записи
        pthread_mutex_lock(&lock_writer);

        // Запись в текущую ячейку и переход на следующую
        buffer->store[current_index] = counter;
        current_index = (current_index + 1) % BUF_SIZE;
        printf("Producer %d writes value = %d to cell [%d]\\n", getpid(), counter, current_index - 1 < 0 ? BUF_SIZE - 1 : current_index - 1);

        // Установка pid процесса-писателя
        buffer->writer_pid = getpid();

        // Увеличение счетчика записанных ячеек
        counter++;

        // Выход из критической секции
        pthread_mutex_unlock(&lock_writer);

        // Освобождение ячейки буфера
        if (sem_post(full) == -1) {
            perror("sem_post: Can not increment full semaphore");
            exit(-1);
        };

        // Проверка на окончание работы
        if (counter == BUF_SIZE) {
            printf("All cells have been written, writer %d exits\\n", getpid());
            exit(0);
        }

        // Разблокирование семафора администратора
        if (sem_post(admin) == -1) {
            perror("sem_post: Can not increment admin semaphore");
            exit(-1);
        }
    }

    return 0;
}
