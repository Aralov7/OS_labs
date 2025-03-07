#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

typedef float (*tDerivative)(float, float);
typedef int (*tGCF)(int, int);

typedef struct {
    tGCF GCF;
    tDerivative Derivative;
    void* library;
} tFuncLibrary;

tFuncLibrary load_library(char* filename) {
    // Загрузка библиотеки
    tFuncLibrary result;
    result.library = dlopen(filename, RTLD_LAZY); // RTLD_LAZY - отложенная загрузка
    if (!result.library) {
        fprintf(stderr, "Ошибка загрузки библиотек: %s\n", dlerror());
        return result;
    }

    // Загрузка функций
    result.Derivative = dlsym(result.library, "Derivative");
    result.GCF = dlsym(result.library, "GCF");

    if (!result.Derivative || !result.GCF) { //не возвращают ли NULL
        fprintf(stderr, "Ошибка загрузки функций из библиотеки: %s\n", dlerror());
        dlclose(result.library);
        result.library = NULL;
        return result;
    }

    return result;
}


int main() {
    tFuncLibrary funcLib = load_library("./libImpl1.so");
    if (funcLib.library == NULL) {
        return 1;
    }
    int lib_index = 0;

    int command;
    while (1) {
        printf("Input program code:\n");
        printf(" 0 -> Library switch\n");
        printf(" 1 -> Derivative\n");
        printf(" 2 -> GCF\n");
        printf("-1 -> Exit\n");
        scanf("%d", &command);

        switch (command) {
            case -1: // Выход из программы
                dlclose(funcLib.library);
                return 0;

            case 0: // Переключение библиотеки
                dlclose(funcLib.library);
                lib_index = lib_index == 0 ? 1 : 0;
                funcLib = load_library(lib_index == 0 ? "./libImpl1.so" : "./libImpl2.so");
                if (funcLib.library == NULL) {
                    continue;
                }

                printf("Library switched successfully!\n");
                printf("Current lib: %s\n", lib_index == 0 ? "./libImpl1.so" : "./libImpl2.so");
                break;

            case 1: // Вычисление производной
                {
                    float A, deltaX;
                    printf("Enter A and deltaX: ");
                    scanf("%f %f", &A, &deltaX); 
                    printf("Derivative(%f, %f) = %f\n", A, deltaX, funcLib.Derivative(A, deltaX));
                    printf("Implementation used: %s\n", lib_index == 0 ? "./libImpl1.so" : "./libImpl2.so");
                }
                break;

            case 2: // Вычисление НОД
                {
                    int A, B;
                    printf("Enter A and B: ");
                    scanf("%d %d", &A, &B);
                    printf("gcd(%d, %d) = %d\n", A, B, funcLib.GCF(A, B));
                    printf("Implementation used: %s\n", lib_index == 0 ? "./libImpl1.so" : "./libImpl2.so");
                }
                break;

            default: // Некорректная команда
                printf("Invalid command\n");
                break;
        }
    }

    dlclose(funcLib.library);
    return 0;
}