#include <iostream>
#include <string>

#define MINLEVEL 4

class Allocator_Buddy
{
    union Block
    {
        int size;
        Block *next;
    };

private:
    void *buffer; // буффер
    Block **queues;
    int levels;

    bool is_bro(Block* bro1, Block*bro2, int teck_level)
    {
        size_t tbro1 = size_t(bro1) - size_t(buffer);
        size_t tbro2 = size_t(bro2) - size_t(buffer);
    
        size_t mask = 1 << (levels - teck_level);
        mask = ~mask;
        return (tbro1 & mask) == (tbro2 & mask);
    }
    
    int log2_top(size_t x)
    {
        int log2 = 0;
        size_t x2 = x;
        while (x >>= 1)
        {
            log2++;
        }
        if (x2 != static_cast<size_t>(1) << log2)
        {
            log2++;
        }
        return log2;
    }

    int log2_down(size_t x)
    {
        int log2 = 0;
        while (x >>= 1)
        {
            log2++;
        }
        return log2;
    }

public:
    Allocator_Buddy(void *realMemory, size_t memory_size)
    {
        levels = log2_down(memory_size);
        int q_mem = sizeof(Block) * (levels - MINLEVEL + 1);
        levels = log2_down(memory_size - q_mem);
        queues = (Block **)realMemory;
    
        buffer = (char *)realMemory + q_mem;
        queues[0] = (Block *)buffer;
        queues[0]->next = nullptr;
        for (int i = 1; i <= levels - MINLEVEL; ++i)
            queues[i] = nullptr;
    }

    void *alloc(size_t block_size)
    {
        if (levels < log2_top(block_size + sizeof(Block))) // если попросили больше чем размер аллокатора
            return nullptr;
        size_t required_level = levels - log2_top(block_size + sizeof(Block)); // определяем какой уровень нам нужен
        if (required_level > static_cast<size_t>(levels - MINLEVEL))           // округляем маленький запрос до размера минимального блока в аллокаторе
            required_level = levels - MINLEVEL;
    
        if (queues[required_level] != nullptr) // в нужной очередь есть подходящий блок
        {
            Block *ans = (Block *)queues[required_level];
            queues[required_level] = queues[required_level]->next;
            ans->size = required_level;
            return ans + 1;
        }
        int i = required_level - 1;
        // не повезло, находим минимальный  непустой уровень
        for (int j = i; j >= -1; j--)
        {
            if (queues[j] != nullptr){ // опааа, нашлии очередь с блоком свободным
                i = j; 
                break;
            }    
            if (j == -1)        // нет свободных блоков
                return nullptr; // памяти нет :(
        }    

        while (queues[required_level] == nullptr) // пока в нужной очереди не появились блоки делим более большие
        {
            Block *big = queues[i]; // убираем из очереди голову
            queues[i] = queues[i]->next;
    
            Block *bro1, *bro2;
            bro1 = big;
            bro2 = (Block *)((char *)big + (1 << (levels - i)) / 2);
    
            bro1->next = bro2;
            bro2->next = queues[i + 1];
            queues[i + 1] = bro1;

    
            ++i;
        }
    
        // вышли из цикла, значит доделили что в нашей очереди есть блоки. Возвращаем его
        Block *ans = (Block *)queues[required_level];
        queues[required_level] = queues[required_level]->next;
        ans->size = required_level;
        return ans + 1;
    }

    void free(void *block)
    {
        if (block == nullptr)
            return;
        Block *cur = (Block *)block - 1; // поняли в каком мы блоке сейчас находимся
        int teck_level = cur->size;      // нашли текущий уровень
    
        cur->next = queues[teck_level];
        queues[teck_level] = cur; // добавили в очередь свободных блоков нужного размера
    
        // теперь проходимся вверх и пытаемся соеденить cur с двойником
        bool bro_find = true;
        while (bro_find && teck_level >= 0)
        {
            bro_find = false;
            cur = queues[teck_level];
            Block *prev = queues[teck_level];

            while (prev != nullptr)
            {
                if (is_bro(cur, prev->next, teck_level))
                {
                    bro_find = true;
                    Block *uni = std::min(prev->next, cur);
                    prev->next = prev->next->next;
                    queues[teck_level] = cur->next;
    
                    uni->next = queues[teck_level - 1];
                    queues[teck_level - 1] = uni;
                    break;
                }
                prev = prev->next;
            }
            --teck_level;
        }
    }

    size_t get_free_memory()
    {
        long ans = 0;
        for (int i = 0; i < levels - MINLEVEL; ++i)
        {
            Block *cur = queues[i];
            while (cur != nullptr)
            {
                ans += (1 << (levels - i));
                cur = cur->next;
            }
        }
        return ans;
    }
    
};