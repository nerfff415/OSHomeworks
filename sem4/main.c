#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define BUFFER_SIZE 10

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <input_file> <output_file>\n", argv[0]);
        return 1;
    }

    int input_fd, output_fd;
    ssize_t read_bytes, write_bytes;
    char buffer[BUFFER_SIZE];

    // Открываем файл для чтения
    input_fd = open(argv[1], O_RDONLY);
    if (input_fd == -1) {
        perror("Failed to open input file");
        return 1;
    }

    // Создаем файл для записи
    output_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (output_fd == -1) {
        perror("Failed to create output file");
        close(input_fd);
        return 1;
    }

    // Читаем из входного файла и записываем в выходной файл
    while ((read_bytes = read(input_fd, buffer, BUFFER_SIZE)) > 0) {
        write_bytes = write(output_fd, buffer, read_bytes);
        if (write_bytes != read_bytes) {
            perror("Failed to write to output file");
            close(input_fd);
            close(output_fd);
            return 1;
        }
    }

    if (read_bytes == -1) {
        perror("Failed to read input file");
        close(input_fd);
        close(output_fd);
        return 1;
    }

    // Закрываем файлы
    close(input_fd);
    close(output_fd);

    return 0;
}
