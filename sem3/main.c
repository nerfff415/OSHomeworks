#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int factorial(int n) {
    if (n == 0 || n == 1)
        return 1;
    else
        return n * factorial(n - 1);
}

int fibonacci(int n) {
    if (n == 0)
        return 0;
    else if (n == 1)
        return 1;
    else
        return fibonacci(n - 1) + fibonacci(n - 2);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./program_name <number>\n");
        return 1;
    }
    int number = atoi(argv[1]);
    pid_t pid = fork();
    if (pid < 0) {
        printf("Fork failed.\n");
        return 1;
    } else if (pid == 0) {
        int factorial_result = factorial(number);
        printf("Factorial of %d is %d\n", number, factorial_result);
    } else {
        int fibonacci_result = fibonacci(number);
        printf("Fibonacci number at position %d is %d\n", number, fibonacci_result);
    }
    return 0;
}
