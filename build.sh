#!/bin/bash
echo "Сборка проекта Linux"
mkdir -p build
cd build
cmake ..
make

echo ""
echo "Запуск тестов"
echo "Тест 1: Без аргументов"
./process_test ./child

echo ""
echo "Тест 2 С аргументами"
./process_test ./child hello world 123

echo ""
echo "Тест 3 (нажми Ctrl+C для проверки)"
./process_test ./child

echo ""
echo "Все тесты завершены"