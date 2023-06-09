#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

pid_t receiver_pid;
int bit_count = 0;
int current_bit = 0;

void send_bit(int bit) {
    if (bit) {
        kill(receiver_pid, SIGUSR2);
    } else {
        kill(receiver_pid, SIGUSR1);
    }
    usleep(100); // Ждем некоторое время для обработки сигнала приемником
}

void send_number(int number) {
    for (int i = 0; i < sizeof(int) * 8; i++) {
        int bit = (number >> i) & 1;
        send_bit(bit);
    }
}

void handle_receiver_pid(int sig) {
    printf("Enter the receiver's PID: ");
    scanf("%d", &receiver_pid);
}

void handle_sigusr1(int sig) {
    current_bit = current_bit | (0 << bit_count);
    bit_count++;
}

void handle_sigusr2(int sig) {
    current_bit = current_bit | (1 << bit_count);
    bit_count++;
}

int main() {
    printf("Transmitter PID: %d\n", getpid());
    signal(SIGUSR1, handle_sigusr1);
    signal(SIGUSR2, handle_sigusr2);

    signal(SIGINT, handle_receiver_pid);
    printf("Enter the receiver's PID: ");
    scanf("%d", &receiver_pid);

    int number;
    printf("Enter an integer number: ");
    scanf("%d", &number);

    send_number(number);

    printf("Transmission completed.\n");

    return 0;
}
