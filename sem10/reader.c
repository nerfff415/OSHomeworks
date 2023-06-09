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
    if ((buf_id = shm_open(shar_object, O_CREAT | O_RDWR, 0666)) == -1) {
        perror("shm_open");
        exit(-1);
    } else {
        printf("Object is open: name = %s, id = 0x%x\\\\n", shar_object, buf_id);
    }
// Задание размера объекта памяти
    if (ftruncate(buf_id, sizeof(shared_memory)) == -1) {
        perror("ftruncate");
        exit(-1);
    } else {
        printf("Memory size set and = %lu\\\\n", sizeof(shared_memory));
    }
// Получить доступ к памяти
    buffer = mmap(0, sizeof(shared_memory), PROT_WRITE | PROT_READ, MAP_SHARED, buf_id, 0);
    if (buffer == (shared_memory *) -1) {
        perror("reader: mmap");
        exit(-1);
    }
    printf("mmap checkout\\\\n");
// Разборки читателей. Мьютекс для конкуренции за работу
    pthread_mutex_init(&lock_reader, NULL);
// Инициализация семафоров
    if ((mutex = sem_open("/mutex", O_CREAT, 0666, 1)) == 0) {
        perror("sem_open: Can not create mutex semaphore");
        exit(-1);
    };
    if ((empty = sem_open("/empty", O_CREAT, 0666, BUF_SIZE)) == 0) {
        perror("sem_open: Can not create empty semaphore");
        exit(-1);
    };
    if ((full = sem_open("/full", O_CREAT, 0666, 0)) == 0) {
        perror("sem_open: Can not create full semaphore");
        exit(-1);
    };
    if ((admin = sem_open("/admin", O_CREAT, 0666, 0)) == 0) {
        perror("sem_open: Can not create admin semaphore");
        exit(-1);
    };
// Подъем администратора, разрешающего продолжение процессов читателя
// После того, как сформировался объект в памяти и прошла его инициализация.
// В начале идет проверка на запуск хотя бы отдного писателя, то есть,
// на то, что семафор уже поднят, и писатели поступают.
// Это позволяет новым писателям не увеличивать значение семафора.
    int is_writers = 0;
    sem_getvalue(admin, &is_writers);
    if (is_writers == 0) {
// Срабатывает при запуске первого писателя
        if (sem_post(admin) == -1) {
            perror("sem_post: Can not increment admin semaphore");
            exit(-1);
        }
    }
// Алгоритм читателя
    while (1) {
// Проверка наличия данных в буфере (ждать если пуст)
        if (sem_wait(full) == -1) { //защита операции чтения
            perror("sem_wait: Incorrect wait of full semaphore");
            exit(-1);
        };
//критическая секция, конкуренция с писателем
        pthread_mutex_lock(&lock_reader);
// Чтение текущей ячейки и переход на следующую
        int value = buffer->store[current_index];
        current_index = (current_index + 1) % BUF_SIZE;
        printf("Consumer %d reads value = %d from cell [%d]\\\\\\\\n",
               getpid(), value, current_index - 1 < 0 ? BUF_SIZE - 1 : current_index - 1);
// Увеличение счетчика прочитанных ячеек
        counter++;
// Если все ячейки прочитаны, завершаем работу
        if (counter == BUF_SIZE) {
            printf("All cells have been read, reader %d exits\\\\\\\\n", getpid());
// Уменьшаем количество читателей
            buffer->have_reader--;
// Если читателей не осталось, то завершаем работу всех процессов
            if (buffer->have_reader == 0 && buffer->have_writer == 0) {
                if (shm_unlink(shar_object) == -1) {
                    perror("shm_unlink");
                    exit(-1);
                }
                printf("All processes have been closed, shared memory has been deleted\\\\\\\\n");
            }
            exit(0);
        }
// Выход из критической секции
        pthread_mutex_unlock(&lock_reader);
// Освобождение ячейки буфера
        if (sem_post(empty) == -1) {
            perror("sem_post: Can not increment empty semaphore");
            exit(-1);
        };
    }
    return 0;
}