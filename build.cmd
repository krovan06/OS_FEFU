   @echo off
   echo Сборка проекта (Windows)
   if not exist build mkdir build
   cd build
   cmake .. -G "MinGW Makefiles"
   mingw32-make
   
   echo.
   echo Запуск тестов
   echo Тест 1: Без аргументов
   process_test.exe child.exe
   
   echo.
   echo Тест 2: С аргументами
   process_test.exe child.exe hello world 123
   
   echo.
   echo Готово
   pause