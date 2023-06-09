#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

int main() {
    char temp_file_name[32], temp_file_name1[32];
    struct stat buffer;

    creat("file.txt", 0666);
    int i = 1;
    sprintf(temp_file_name, "file%d.txt", i);
    symlink("file.txt", temp_file_name);
    for (i = 2; i < 1000; ++i) {
        sprintf(temp_file_name1, "file%d.txt", i);

        symlink(temp_file_name, temp_file_name1);
        int status = stat(temp_file_name1, &buffer);
        if (status < 0) {
            printf("Can\'t create link file %s\n", temp_file_name1);
            printf("Created %d symbol links\n", i);
            return 0;
        }
        sprintf(temp_file_name, "%s", temp_file_name1);
    }

    return 0;
}
