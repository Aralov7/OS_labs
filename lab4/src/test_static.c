#include <stdio.h>

// Объявление функций из библиотек
extern float Derivative(float, float);
extern int GCF(int, int);

int main() {
    int command;
    while (1) {
        printf("Input program code:\n");
        printf(" 1 -> Derivative\n");
        printf(" 2 -> GCF\n");
        printf("-1 -> Exit\n");
        scanf("%d", &command);

        switch (command) {
            case -1: // Выход из программы
                return 0;

            case 1: { // Вычисление производной
                float A, deltaX;
                printf("Enter A and deltaX: ");
                scanf("%f %f", &A, &deltaX);
                printf("Derivative(%f, %f) = %f\n", A, deltaX, Derivative(A, deltaX));
                break;
            }

            case 2: { // Вычисление НОД
                int A, B;
                printf("Enter A and B: ");
                scanf("%d %d", &A, &B);
                printf("gcd(%d, %d) = %d\n", A, B, GCF(A, B));
                break;
            }

            default: // Некорректная команда
                printf("Invalid command\n");
                break;
        }
    }
    return 0;
}