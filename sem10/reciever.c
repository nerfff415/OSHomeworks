#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

pid_t transmitter_pid;
int bit_count = 0;
int received_number = 0;

void handle_transmitter_pid(int sig) {
    printf("Enter the transmitter's PID: ");
    scanf("%d", &transmitter_pid);
}

void handle_sigusr1(int sig) {
    received_number = received_number | (0 << bit_count);
    bit_count++;
}

void handle_sigusr2(int sig) {
    received_number = received_number | (1 << bit_count);
    bit_count++;
}

int main() {
    printf("Receiver PID: %d\n", getpid());
    signal(SIGUSR1, handle_sigusr1);
    signal(SIGUSR2, handle_sigusr2);

    signal(SIGINT, handle_transmitter_pid);
    printf("Enter the transmitter's PID: ");
    scanf("%d", &transmitter_pid);

    printf("Waiting for transmission...\n");

    while (bit_count < sizeof(int) * 8) {
        sleep(1);
    }

    printf("Received number: %d\n", received_number);

    return 0;
}
