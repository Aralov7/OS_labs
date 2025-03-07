//main.c

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>



typedef struct {
    double x, y, z;
    
} Point;

typedef struct {
    Point *array;
    int n;
    int thread;
    int arraysize;
} ThreadData;

typedef struct {
    Point array[3];
    double area;
} ExitData;

double triangleArea(Point A, Point B, Point C) {
    const double epsilon = 1e-9; // Точность для проверки коллинеарности
    
    // Вычисляем векторы AB и AC
    Point AB = {B.x - A.x, B.y - A.y, B.z - A.z};
    Point AC = {C.x - A.x, C.y - A.y, C.z - A.z};
    
    // Вычисляем векторное произведение AB × AC
    Point cross;
    cross.x = AB.y * AC.z - AB.z * AC.y;
    cross.y = AB.z * AC.x - AB.x * AC.z;
    cross.z = AB.x * AC.y - AB.y * AC.x;
    
    // Вычисляем длину вектора произведения
    double length = sqrt(cross.x * cross.x + cross.y * cross.y + cross.z * cross.z);
    // Если точки коллинеарны (площадь нулевая)
    if (length < epsilon) {
        return 0.0;
    }
    
    return length / 2.0;
}

//поиск maxArea, выполняемая в потоке
void *maxAreaThread(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    ExitData exit_data;
    Point *arr = data -> array;
    int step = data -> n;
    int i = data -> thread;
    int size = data -> arraysize;
    
    double max = 0;
    for (i; i < size - 2; i += step){
        for (int j = i + 1; j < size - 1; j++){
            for (int k = j + 1; k < size; k++)
            {       
               if (triangleArea(arr[i], arr[j], arr[k]) > max){
                exit_data.area = triangleArea(arr[i], arr[j], arr[k]);
                exit_data.array[0] = arr[i];
                exit_data.array[1] = arr[j];
                exit_data.array[2] = arr[k];
               }
            }
        }
    }

    ExitData *exit_data_ptr = malloc(sizeof(ExitData));
    *exit_data_ptr = exit_data;
    pthread_exit((void*)exit_data_ptr);
}

//основная ф-я maxArea
ExitData maxArea(Point arr[], int n, int maxThreads) {

    pthread_t *threads = (pthread_t *)malloc(maxThreads * sizeof(pthread_t));
    ThreadData *threadData = (ThreadData *)malloc(maxThreads * sizeof(ThreadData));

    for (int i = 0; i < maxThreads; i++) {
        threadData[i].array = arr;
        threadData[i].n = maxThreads;
        threadData[i].thread = i;
        threadData[i].arraysize = n;
        
        pthread_create(&threads[i], NULL, maxAreaThread, (void *)&threadData[i]);
    }

    // Массив для хранения результатов
    ExitData *exit_array = (ExitData *)malloc(maxThreads * sizeof(ExitData));

    // Ожидаем завершения потоков и сохраняем результаты
    for (int i = 0; i < maxThreads; i++) {
        void *exit_data;
        pthread_join(threads[i], &exit_data); // Получаем указатель на ExitData
        exit_array[i] = *(ExitData *)exit_data; // Копируем данные
        free(exit_data); // Освобождаем память, выделенную в потоке
    }

    double result = 0;
    int index = -1;
    for (int i = 0; i < maxThreads; i++)
    {
        if (exit_array[i].area > result){ 
            result = exit_array[i].area;
            index = i;
    }
    }

    free(threads);
    free(threadData);

    return exit_array[index];
}


int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <max_threads>\n", argv[0]);
        return 1;
    }

    int maxThreads = atoi(argv[1]);
    int arraySize;

    printf("Enter the number of elements in the array: ");
    scanf("%d", &arraySize);

    Point *arr = (Point *)malloc(arraySize * sizeof(Point));

    // Инициализация генератора случайных чисел
    srand(time(NULL));

    // Генерация случайных чисел для массива
    for (int i = 0; i < arraySize; i++) {
        arr[i].x = rand() % 100; // Генерация случайных чисел от 0 до 99
        arr[i].y = rand() % 100;
        arr[i].z = rand() % 100;
    }

    
    // printf("Original array:\n");
    // for (int i = 0; i < arraySize; i++) {
    //     printf("{%.2f, %.2f, %.2f}\n", arr[i].x, arr[i].y, arr[i].z);
    // }
    // printf("\n");

    struct timeval start, end;
    gettimeofday(&start, NULL);

    ExitData otvet = maxArea(arr, arraySize, maxThreads);

    gettimeofday(&end, NULL);

    for (int i = 0; i < 3; i++)
    {
        printf("{%.2f, %.2f, %.2f} - Point %d\n", otvet.array[i].x, otvet.array[i].y, otvet.array[i].z, i);
        
    
    }printf("%f - Max area\n", otvet.area);

    long seconds = end.tv_sec - start.tv_sec;
    long microseconds = end.tv_usec - start.tv_usec;
    double elapsed = seconds + microseconds * 1e-6;

    printf("Time taken for calculating: %.6f seconds\n", elapsed);
    printf("Number of threads used: %d\n", maxThreads);

    free(arr);
    return 0;
}