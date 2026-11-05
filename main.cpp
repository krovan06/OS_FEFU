#include <iostream>
#include "process.h"

#include <csignal>
#include <cstdlib>

using namespace std;

// Глобальная переменная для хранения PID ребенка, чтобы убить его при Ctrl+C
static Process g_child_process = 0;

// Обработчик сигнала (РЕШЕНИЕ ЗАМЕЧАНИЯ ПРО Ctrl+C)
extern "C" void signal_handler(int signum) {
    if (g_child_process > 0) {
        cout << "\nРодитель: получен сигнал (Ctrl+C). Завершаем дочерний процесс..." << endl;
        terminate_program(g_child_process);
    }
    exit(1);
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    if (argc < 2) {
        cout << "Использование: process_test <путь_к_программе> [аргументы...]" << endl;
        return 1;
    }

    // Настраиваем перехват Ctrl+C
    signal(SIGINT, signal_handler);

    // Формируем аргументы для дочернего процесса
    ProcessArgs args;
    if (argc > 2) {
        const char** argv_array = new const char*[argc - 2];
        for (int i = 2; i < argc; ++i) {
            argv_array[i - 2] = argv[i];
        }
        args.argv = argv_array;
        args.count = argc - 2;
    }

    // Настраиваем опции: ребенок умрет, если умрет родитель
    ProcessOptions options;
    options.kill_on_parent_exit = true;

    cout << "Запуск: " << argv[1] << endl;
    
    if (!start_program(argv[1], g_child_process, &args, &options)) {
        cout << "Ошибка: не удалось запустить программу" << endl;
        if (args.argv != NULL) delete[] args.argv;
        return 1;
    }

    cout << "Родитель: программа запущена (PID: " << g_child_process << "). Ожидание..." << endl;

    int code;
    if (!wait_program(g_child_process, code)) {
        cout << "Ошибка: не удалось получить код завершения" << endl;
        if (args.argv != NULL) delete[] args.argv;
        return 1;
    }

    cout << "Родитель: код завершения дочерней программы = " << code << endl;

    if (args.argv != NULL) delete[] args.argv;
    return 0;
}
