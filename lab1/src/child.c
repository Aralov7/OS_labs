#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>

#define MAX_SIZE 100

int sum_array(int *array, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += array[i + 1];
    }
    return sum;
}

int main() {
    int sum, numbers[MAX_SIZE];

    // Чтение числа из стандартного ввода
    if (read(STDIN_FILENO, &numbers, sizeof(numbers)) == -1) {
        fprintf(stderr, "Ошибка при чтении чисел\n");
        return 1;
    }
    sum = sum_array(numbers, numbers[0]);

    int file = open("result.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file == -1) {
        fprintf(stderr, "Ошибка при открытии файла\n");
        return 1;
    }

    // Форматирование результата в строку
    char buffer[16];
    int n = snprintf(buffer, sizeof(buffer), "%d\n", sum);
    if (n <= 0) {
        perror("Ошибка при форматировании строки");
        close(file);
        return 1;
    }

    // Запись строки в файл
    if (write(file, buffer, n) == -1) {
        fprintf(stderr, "Ошибка при записи в файл\n");
        close(file);
        return 1;
    }

    close(file);


    return 0;
}