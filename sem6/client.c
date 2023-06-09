#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <unistd.h>

#define SHM_KEY 1234

typedef struct {
    int data;
    int done;
} SharedData;

int main() {
    int shm_id;
    SharedData* shared_data;
    shm_id = shmget(SHM_KEY, sizeof(SharedData), IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("shmget");
        exit(1);
    }
    shared_data = (SharedData*)shmat(shm_id, NULL, 0);
    if (shared_data == (SharedData*)(-1)) {
        perror("shmat");
        exit(1);
    }
    srand(getpid());
    shared_data->data = rand() % 100;
    shared_data->done = 1;
    shmdt(shared_data);
    return 0;
}
