

// reader.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>

#define BUF_SIZE 10

typedef struct {
    int store[BUF_SIZE];
    int have_reader;
    int have_writer;
    pid_t writer_pid;
} shared_memory;

int main(int argc, char *argv[]) {
    char *shar_object = "/my_shared_memory";
    char *reader_sem_name = "/reader_semaphore";
    char *admin_sem_name = "/admin_semaphore";
    sem_t *reader, *mutex, *empty, *full, *admin;
    int buf_id;
    shared_memory *buffer;
    if ((admin = sem_open(admin_sem_name, O_CREAT, 0666, 0)) == 0) {
        perror("sem_open: Can not create admin semaphore");
        exit(-1);
    }
    // Получение доступа к кольцевому буферу
    if ((buf_id = shm_open(shar_object, O_RDWR, 0666)) == -1) {
        perror("shm_open");
        exit(-1);
    } else {
        printf("Object is open: name = %s, id = 0x%x\\\\n", shar_object, buf_id);
    }
    // Получить доступ к памяти
    buffer = mmap(0, sizeof(shared_memory), PROT_WRITE | PROT_READ, MAP_SHARED, buf_id, 0);
    if (buffer == (shared_memory *) -1) {
        perror("reader: mmap");
        exit(-1);
    }
    printf("mmap checkout\\\\n");
    // Разборки читателя
    if ((reader = sem_open(reader_sem_name, O_CREAT, 0666, 1)) == 0) {
        perror("sem_open: Can not create reader semaphore");
        exit(-1);
    };
    // Инициализация счетчиков
    int current_index = 0;
    int counter = 0;
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
        if (sem_wait(mutex) == -1) {
            perror("sem_wait: Incorrect wait of mutex");
            exit(-1);
        };
        // Чтение текущей ячейки и переход на следующую
        int value = buffer->store[current_index];
        current_index = (current_index + 1) % BUF_SIZE;
        printf("Consumer %d reads value = %d from cell [%d]\\\\n",
               getpid(), value, current_index - 1 < 0 ? BUF_SIZE - 1 : current_index - 1);
        // Увеличение счетчика прочитанных ячеек
        counter++;
        // Если все ячейки прочитаны, завершаем работу
        if (counter == BUF_SIZE) {
            printf("All cells have been read, reader %d exits\\\\n", getpid());
            exit(0);
        }
        // Выход из критической секции
        if (sem_post(mutex) == -1) {
            perror("sem_post: Incorrect post of mutex semaphore");
            exit(-1);
        };
        // Освобождение ячейки буфера
        if (sem_post(empty) == -1) {
            perror("sem_post: Incorrect post of empty semaphore");
            exit(-1);
        };
        sleep(rand() % 3 + 1);
    }
    return 0;
}

