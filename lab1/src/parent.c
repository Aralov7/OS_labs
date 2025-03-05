#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_SIZE 100
int main(){
    int fd1[2], fd2[2];

    if (pipe(fd1) == -1 || pipe(fd2) == -1) {
        printf("Ошибка при создании pipe\n");
        return 1;
    }

    pid_t pid = fork();
    if (pid == -1) {
        printf("Ошибка при создании дочернего процесса\n");
        return 1;
    }

    if (pid > 0) { // Родительский процесс
        close(fd1[0]); // Закрываем чтение из первого pipe
        close(fd2[1]); // Закрываем запись во второй pipe
    
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

        if (write(fd1[1], &numbers, sizeof(numbers)) == -1) {
            printf("Ошибка при передаче числа\n");
            return 1;
        }
    
        // Ожидание завершения дочернего процесса
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status) && WEXITSTATUS(status) == 1) {
            fprintf(stderr, "Дочерний процесс завершил работу с ошибкой.\n");
            return 1;
        }
        close(fd1[1]); // Закрываем запись в первый pipe
        close(fd2[0]); // Закрываем чтение из второго pipe
    } else { // Дочерний процесс
        close(fd1[1]); // Закрываем запись в первый pipe
        close(fd2[0]); // Закрываем чтение из второго pipe

        // Перенаправление stdin и stdout на pipe
        if (dup2(fd1[0], STDIN_FILENO) == -1) {
            printf("Ошибка при перенаправлении stdin\n");
            return 1;
        }
        if (dup2(fd2[1], STDOUT_FILENO) == -1) {
            printf("Ошибка при перенаправлении stdout\n");
            return 1;
        }

        // Запуск дочернего процесса
        execl("./child", "child", NULL);
        printf("Ошибка при вызове execl\n");
        return 1;
    }
    printf("Программа завершила работу без ошибок\n");
    return 0;
}