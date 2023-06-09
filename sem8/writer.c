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
    char *writer_sem_name = "/writer_semaphore";
    char *first_writer_sem_name = "/first_writer_semaphore";
    char *admin_sem_name = "/admin_semaphore";
    sem_t *writer, *first_writer, *mutex, *empty, *full, *admin;
    int buf_id;
    shared_memory *buffer;
    if ((admin = sem_open(admin_sem_name, O_CREAT, 0666, 0)) == 0) {
        perror("sem_open: Can not create admin semaphore");
        exit(-1);
    }
    // Получение доступа к кольцевому буферу
    if ((buf_id = shm_open(shar_object, O_CREAT | O_RDWR, 0666)) == -1) {
        perror("shm_open");
        exit(-1);
    } else {
        printf("Object is open: name = %s, id = 0x%x\\n", shar_object, buf_id);
    }
    // Задание размера объекта памяти
    if (ftruncate(buf_id, sizeof(shared_memory)) == -1) {
        perror("ftruncate");
        exit(-1);
    } else {
        printf("Memory size set and = %lu\\n", sizeof(shared_memory));
    }
    // Получить доступ к памяти
    buffer = mmap(0, sizeof(shared_memory), PROT_WRITE | PROT_READ, MAP_SHARED, buf_id, 0);
    if (buffer == (shared_memory *) -1) {
        perror("writer: mmap");
        exit(-1);
    }
    printf("mmap checkout\\n");
    // Разборки писателей. Семафор для конкуренции за работу
    if ((writer = sem_open(writer_sem_name, O_CREAT, 0666, 1)) == 0) {
        perror("sem_open: Can not create writer semaphore");
        exit(-1);
    };
    // Дополнительный семафор, играющий роль счетчика-ограничителя
    if ((first_writer = sem_open(first_writer_sem_name, O_CREAT, 0666, 1)) == 0) {
        perror("sem_open: Can not create first_writer semaphore");
        exit(-1);
    };
    // Первый просочившийся запрещает доступ остальным за счет установки флага
    if (sem_wait(writer) == -1) {
        perror("sem_wait: Incorrect wait of writer semaphore");
        exit(-1);
    };
    // Он проверяет значение семафора, к которому другие не имеют доступ
    // и устанавливает его в 0, если является первым писателем.
    // Остальные писатели при проверке завершают работу
    int writer_number = 0;
    sem_getvalue(first_writer, &writer_number);
    printf("checking: writer_number = %d\\n", writer_number);
    // Остальные завершают работу по единичному флагу.
    if (writer_number == 0) {
        // Завершение процесса
        printf("Writer %d: I have lost this work :(\\n", getpid());
        // Безработных писателей может быть много. Нужно их тоже отфутболить.
        // Для этого они должны войти в эту секцию
        if (sem_post(writer) == -1) {
            perror("sem_post: Consumer can not increment writer semaphore");
            exit(-1);
        }
        exit(13);
    }
    // Первый просочившийся запрещает доступ остальным
    // за счет блокирования этого семафора. Они до этого кода не должны дойти
    if (sem_wait(first_writer) == -1) {
        perror("sem_wait: Incorrect wait of first_writer semaphore");
        exit(-1);
    };
    // Пропуск других писателей для определения возможности поработать
    if (sem_post(writer) == -1) {
        perror("sem_post: Consumer can not increment writer semaphore");
        exit(-1);
    }
    // сохранение pid для корректного взаимодействия с писателем
    buffer->writer_pid = getpid();
    printf("Writer %d: I am first for this work! :)\\n", getpid());
    // Инициализация буфера отрицательными числам, имитирующими пустые ячейки
    // Перед доступом читателя, который обрабатывает натуральные числа в ограниченном диапазоне
    for (int i = 0; i < BUF_SIZE; ++i) {
        buffer->store[i] = -1;
    }
    buffer->have_reader = 0;
    // buffer->have_writer = 0;
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
    
    int current_index = 0;
    // Алгоритм писателя
    while (1) {
        // Проверка заполнения буфера (ждать если полон)
        if (sem_wait(empty) == -1) { //защита операции записи
            perror("sem_wait: Incorrect wait of empty semaphore");
            exit(-1);
        };
        //критическая секция, конкуренция с читателем
        if (sem_wait(mutex) == -1) {
            perror("sem_wait: Incorrect wait of mutex");
            exit(-1);
        };
        // Запись в текущую ячейку буфера
        buffer->store[current_index] = rand() % 11; // число от 0 до 10
        printf("Producer %d writes value = %d to cell [%d]\\n",
               getpid(), buffer->store[current_index], current_index);
        // Переход на следующую ячейку
        current_index = (current_index + 1) % BUF_SIZE;
        //количество занятых ячеек увеличилось на единицу
        if (sem_post(full) == -1) {
            perror("sem_post: Incorrect post of full semaphore");
            exit(-1);
        };
        // Выход из критической секции
        if (sem_post(mutex) == -1) {
            perror("sem_post: Incorrect post of mutex semaphore");
            exit(-1);
        };
        sleep(rand() % 3 + 1);
    }
    return 0;
}
