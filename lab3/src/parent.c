#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#define MAX_SIZE 100

struct SharedData { // Структура передаваеммых данных
    int numbers[MAX_SIZE];
    char message[256];
};

int main() {
    // Размер общей памяти
    size_t shared_size = sizeof(struct SharedData);


    int fd = shm_open("/my_shared_memory", O_CREAT | O_RDWR, 0666); // Создаем файловый дискриптор под shm
    if (fd == -1) {
        perror("Ошибка при создании разделяемой памяти");
        return 1;
    }
    ftruncate(fd, shared_size); // Изменяем размер дискриптора под SharedData

    // Отображение памяти
    struct SharedData* shared_data = (struct SharedData*)mmap(NULL, shared_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    // NULL: Желаемый адрес подключения к адресному пространству, если NULL ядро само выберет подходящий адрес

    // PROT_READ: Разрешает читать

    // PROT_WRITE: Разрешает писать

    // MAP_SHARED: Изменения в отображенной области будут видны другим процессам,
    // которые также отобразили этот файл, и будут записаны обратно в файл.
    
    // fd: Файловый дескриптор, который указывает на файл, который нужно отобразить в память.
    // Этот файл должен быть открыт перед вызовом mmap.

    // 0: Смещение в файле

    if (shared_data == MAP_FAILED) {
        perror("Ошибка при отображении разделяемой памяти");
        return 1;
    }


    // Создание именнованых семафоров
    sem_t* sem_parent = sem_open("/sem_parent", O_CREAT, 0666, 0); //Семафор управляется ядром ОС, и его состояние 
    sem_t* sem_child = sem_open("/sem_child", O_CREAT, 0666, 0);   //(например, значение счетчика) хранится в ядре.
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
        // memcpy копирует массив

        sem_post(sem_child); // Увеличивает значение семафора на 1

        // Ожидание результата от дочернего процесса
        sem_wait(sem_parent); // If семафор == 0: ждет пока он не станет >= 1, else меньшает на 1
        printf("Результат: %s\n", shared_data->message);

        // Завершаем работу
        waitpid(pid, NULL, 0);
        // waitpid(pid_t pid, int *status, int options);

        // Удаление ресурсов
        munmap(shared_data, shared_size);
        sem_close(sem_parent);
        sem_close(sem_child);
        sem_unlink("/sem_parent");
        sem_unlink("/sem_child");
        close(fd);
        shm_unlink("/my_shared_memory");
        
        
    } else { // Дочерний процесс
        execl("./child.o", "child", NULL);
        perror("Ошибка при вызове дочернего процесса");
        return 1;
    }

    return 0;
}