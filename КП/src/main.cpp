#include <iostream>
#include <chrono>
#include "BF.cpp"
#include "Buddy.cpp"

template <typename Allocator>
void test_allocator(Allocator& allocator, const std::string& name, size_t total_memory) {
    const int ITERATIONS = 10000;
    const size_t BLOCK_SIZE = 64;
    void* blocks[ITERATIONS];
    size_t allocated_count = 0;

    // Тестируем выделение памяти
    auto start_alloc = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERATIONS; ++i) {
        blocks[i] = allocator.alloc(BLOCK_SIZE);
        if (blocks[i]) allocated_count++;
    }
    auto end_alloc = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> alloc_time = end_alloc - start_alloc;

    // Вычисляем фактор использования
    size_t current_free = allocator.get_free_memory();
    size_t used_memory = total_memory - current_free;
    double usage_factor = static_cast<double>(used_memory) / total_memory;

    // Тестируем освобождение памяти
    auto start_free = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERATIONS; ++i) {
        if (blocks[i]) allocator.free(blocks[i]);
    }
    auto end_free = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> free_time = end_free - start_free;

    // Вывод результатов
    std::cout << "=== " << name << " ===\n"
              << "Allocated blocks: " << allocated_count << "/" << ITERATIONS << "\n"
              << "Allocation Time: " << alloc_time.count() << " s\n"
              << "Free Time: " << free_time.count() << " s\n"
              << "Memory Usage Factor: " << usage_factor << "\n\n";
}

int main() {
    const size_t MEMORY_SIZE =  4 * 1024 * 1024; // 4 МБ

    // Выделяем память для аллокаторов
    void* buddy_memory = malloc(MEMORY_SIZE);
    void* BF_memory = malloc(MEMORY_SIZE);

    // Создаем аллокаторы
    Allocator_Buddy buddy_allocator(buddy_memory, MEMORY_SIZE);
    Allocator_BF BF_allocator(BF_memory, MEMORY_SIZE);

    // Тестируем
    test_allocator(buddy_allocator, "Buddy Allocator", MEMORY_SIZE);
    test_allocator(BF_allocator, "BF Allocator", MEMORY_SIZE);

    // Освобождаем память
    free(buddy_memory);
    free(BF_memory);

    return 0;
}