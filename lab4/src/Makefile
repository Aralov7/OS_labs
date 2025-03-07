# Все цели по умолчанию
all: libImpl1.so libImpl2.so test_static test_dynamic

# Сборка динамической библиотеки для lib1
libImpl1.so: math_lib1.o gcd_evclid.o
	gcc -shared -o $@ $^ -lm

# Сборка динамической библиотеки для lib2
libImpl2.so: math_lib2.o gcd_naive.o
	gcc -shared -o $@ $^ -lm

# Компиляция первой библиотеки
math_lib1.o: math_lib1.c
	gcc -fPIC -Wall -Wextra -c $< -o $@

gcd_evclid.o: gcd_evclid.c
	gcc -fPIC -Wall -Wextra -c $< -o $@

# Компиляция второй библиотеки
math_lib2.o: math_lib2.c
	gcc -fPIC -Wall -Wextra -c $< -o $@

gcd_naive.o: gcd_naive.c
	gcc -fPIC -Wall -Wextra -c $< -o $@

# Сборка тестовой программы №1 (статическое использование библиотек)
test_static: test_static.c
	gcc -o $@ $< -L. -lImpl1 -lm -Wl,-rpath,.

# Сборка тестовой программы №2 (динамическая загрузка библиотек)
test_dynamic: test_dynamic.c
	gcc -o $@ $< -ldl

# Очистка сгенерированных файлов
clean:
	rm -f *.o *.so test_static test_dynamic

# Указание целей, которые не являются файлами
.PHONY: all clean