#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#define MAX_SIZE 100

struct SharedData {
    int numbers[MAX_SIZE];
    char message[256];
};

int main() {
    // Размер общей памяти
    size_t shared_size = sizeof(struct SharedData);

    // Создание разделяемой памяти
    int fd = open("/my_shared_memory", O_CREAT | O_RDWR, 0666); //
    if (fd == -1) {
        perror("Ошибка при создании разделяемой памяти");
        return 1;
    }
    ftruncate(fd, shared_size);

    // Отображение памяти
    struct SharedData* shared_data = (struct SharedData*)mmap(NULL, shared_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared_data == MAP_FAILED) {
        perror("Ошибка при отображении разделяемой памяти");
        return 1;
    }

    close(fd);

    // Создание семафоров
    sem_t* sem_parent = sem_open("/sem_parent", O_CREAT, 0666, 0);
    sem_t* sem_child = sem_open("/sem_child", O_CREAT, 0666, 0);
    if (sem_parent == SEM_FAILED || sem_child == SEM_FAILED) {
        perror("Ошибка при создании семафоров");
        return 1;
    }

    // Создание дочернего процесса
    pid_t pid = fork();
    if (pid < 0) {
        perror("Ошибка при создании дочернего процесса");
        return 1;
    }

    if (pid > 0) { // Родительский процесс
        int number, numbers[MAX_SIZE];
        int size = 0;
        char buffer[1024]; // Буфер для чтения строки

        printf("Введите данные в формате: «число число число<endline>».\n");
        
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            fprintf(stderr, "Ошибка чтения ввода\n");
            return 1;
        }

        char *token = strtok(buffer, " "); // Разделение строки на токены
        while (token != NULL && size < MAX_SIZE) {
            numbers[++size] = atoi(token); // Преобразование токена в число и сохранение в массив
            token = strtok(NULL, " ");
        }
        numbers[0] = size;

        // Запись чисел в разделяемую память
        memcpy(shared_data->numbers, numbers, sizeof(numbers));

        sem_post(sem_child); // Уведомляем дочерний процесс

        // Ожидание результата от дочернего процесса
        sem_wait(sem_parent);
        printf("Результат: %s\n", shared_data->message);

        // Завершаем работу
        waitpid(pid, NULL, 0);

        // Удаление ресурсов
        munmap(shared_data, shared_size);
        sem_close(sem_parent);
        sem_close(sem_child);
        sem_unlink("/sem_parent");
        sem_unlink("/sem_child");
        
    } else { // Дочерний процесс
        execl("./child.o", "child", NULL);
        perror("Ошибка при вызове дочернего процесса");
        return 1;
    }

    return 0;
}