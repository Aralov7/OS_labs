#include <stdio.h>
#include <stdlib.h>

int GCF(int a, int b) {
    a = abs(a); // НОД для неотрицательных чисел
    b = abs(b);
    
    while (b != 0) {
        int remainder = a % b; // Остаток от деления
        a = b;
        b = remainder;
    }
    return a; // Когда b == 0, НОД = a
}