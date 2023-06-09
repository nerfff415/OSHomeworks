#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>

#define SHM_KEY 1234

typedef struct {
    int data;
    int done;
} SharedData;

int main() {
    int shm_id;
    SharedData* shared_data;
    shm_id = shmget(SHM_KEY, sizeof(SharedData), 0666);
    if (shm_id == -1) {
        perror("shmget");
        exit(1);
    }
    shared_data = (SharedData*)shmat(shm_id, NULL, 0);
    if (shared_data == (SharedData*)(-1)) {
        perror("shmat");
        exit(1);
    }
    while (shared_data->done != 1) {
        sleep(1);
    }
    printf("Received data: %d\n", shared_data->data);
    shmdt(shared_data);
    shmctl(shm_id, IPC_RMID, NULL);
    return 0;
}
