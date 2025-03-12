# Компилятор
CXX = g++

# Флаги компиляции
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

# Исходные файлы
SRCS = main.cpp

# Имя исполняемого файла
TARGET = allocator_test.o


# Правило по умолчанию
all: $(TARGET)

# Сборка исполняемого файла
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

# Очистка
clean:
	rm -f $(TARGET)

# Запуск теста
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run