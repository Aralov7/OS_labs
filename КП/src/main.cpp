#include <iostream>
#include <chrono>
#include <vector>
#include <cstdlib>
#include <random>

#include "BF.cpp"
#include "Buddy.cpp"

const size_t MEMORY_SIZE = 4 * 1024 * 1024; // 4 MB
const int NUM_OPERATIONS = 10000;        // Количество операций
const int MAX_BLOCK_SIZE = 128;         // Максимальный размер блока

// Общая функция для тестирования любого аллокатора
template<typename Allocator>
void test_allocator(const std::string& name, void* memory) {
    Allocator allocator(memory, MEMORY_SIZE);
    std::vector<void*> pointers;
    std::mt19937 rng(42);  // Фиксированный seed для воспроизводимости
    std::uniform_int_distribution<size_t> size_dist(1, MAX_BLOCK_SIZE);

    // Тест скорости выделения памяти
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_OPERATIONS; ++i) {
        void* ptr = allocato йr.alloc(size_dist(rng));
        pointers.push_back(ptr);
    }
    auto duration = std::chrono::high_resolution_clock::now() - start;
    std::cout << "[" << name << "] Allocation: "
              << std::chrono::duration<double>(duration).count() << " sec\n";

    // Тест скорости освобождения памяти
    start = std::chrono::high_resolution_clock::now();
    for (void* ptr : pointers) {
        allocator.free(ptr);
    }
    duration = std::chrono::high_resolution_clock::now() - start;
    std::cout << "[" << name << "] Free:      "
              << std::chrono::duration<double>(duration).count() << " sec\n";

    // Тест эффективности использования памяти
    size_t total_allocated = 0;
    pointers.clear();
    for (int i = 0; i < NUM_OPERATIONS; ++i) {
        size_t size = size_dist(rng);
        if (void* ptr = allocator.alloc(size)) {
            total_allocated += size;
            pointers.push_back(ptr);
        }
    }
    
    const size_t free_mem = allocator.get_free_memory();
    const double utilization = total_allocated / static_cast<double>(MEMORY_SIZE - free_mem);
    
    std::cout << "[" << name << "] Utilization: " 
              << std::fixed << utilization * 100 << "%\n\n";

    // Очистка памяти
    for (void* ptr : pointers) {
        allocator.free(ptr);
    }
}

int main() {
    // Выделяем память для аллокаторов
    char* bf_memory = new char[MEMORY_SIZE];
    char* buddy_memory = new char[MEMORY_SIZE];

    // Запуск тестов
    test_allocator<Allocator_BF>("Best-Fit", bf_memory);
    test_allocator<Allocator_Buddy>("Buddy", buddy_memory);

    // Освобождение памяти
    delete[] bf_memory;
    delete[] buddy_memory;

    return 0;
}