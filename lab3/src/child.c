#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

#define MAX_SIZE 100

struct SharedData { // Структура передаваеммых данных
    int numbers[MAX_SIZE];
    char message[256];
};

int sum_array(int *array, int size) { // Функция сумирования чисел в массиве
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += array[i + 1];
    }
    return sum;
}

int main() {

    int numbers[MAX_SIZE];


    // Открытие разделяемой памяти
    int fd = shm_open("/my_shared_memory", O_RDWR, 0666);
    if (fd == -1) {
        perror("Ошибка при открытии разделяемой памяти");
        return 1;
    }

    // Отображение памяти
    size_t shared_size = sizeof(struct SharedData);
    struct SharedData* shared_data = (struct SharedData*)mmap(NULL, shared_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared_data == MAP_FAILED) {
        perror("Ошибка при отображении разделяемой памяти");
        return 1;
    }

    close(fd);
    shm_unlink("/my_shared_memory");
    
    // Открытие семафоров
    sem_t* sem_parent = sem_open("/sem_parent", 0);
    sem_t* sem_child = sem_open("/sem_child", 0);
    if (sem_parent == SEM_FAILED || sem_child == SEM_FAILED) {
        perror("Ошибка при открытии семафоров");
        return 1;
    }

    // Ожидание чисел от родительского процесса
    sem_wait(sem_child);

    memcpy(numbers, shared_data->numbers, sizeof(shared_data->numbers));

    int sum = sum_array(numbers, numbers[0]);

    int file = open("result.txt", O_WRONLY | O_CREAT | O_TRUNC, 0666);

    if (file == -1) {
        strncpy(shared_data->message, "Ошибка при открытии файла", sizeof(shared_data->message));
        sem_post(sem_parent);
        munmap(shared_data, shared_size);
        return 1;
    }

    // Форматирование результата в строку
    char buffer[16];
    int n = snprintf(buffer, sizeof(buffer), "%d\n", sum);
    if (n <= 0) {
        
        strncpy(shared_data->message, "Ошибка при форматировании строки", sizeof(shared_data->message));
        close(file);
        sem_post(sem_parent);
        munmap(shared_data, shared_size);
        return 1;
    }

    // Запись строки в файл
    if (write(file, buffer, n) == -1) {
        strncpy(shared_data->message, "Ошибка записи в файл", sizeof(shared_data->message));
        close(file);
        sem_post(sem_parent);
        munmap(shared_data, shared_size);
        return 1;
    }

    strncpy(shared_data->message, "Сумма записана в файл", sizeof(shared_data->message));
    close(file);
    sem_post(sem_parent);
    munmap(shared_data, shared_size); //Снимает отображение
    return 0;
}