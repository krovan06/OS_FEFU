#include <iostream>
#include <csignal>
#include <cstdlib>
#include "process.h"

using namespace std;

// Глобальная переменная для хранения дескриптора/PID ребенка
static Process g_child_process = NULL;

// Обработчик сигнала
extern "C" void signal_handler(int signum) {
#ifdef _WIN32
    // В Windows Process это HANDLE (указатель). Сравниваем с NULL и INVALID_HANDLE_VALUE
    if (g_child_process != NULL && g_child_process != INVALID_HANDLE_VALUE) {
#else
    // В Linux Process это pid_t (целое число). Сравниваем с 0
    if (g_child_process > 0) {
#endif
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

    // Настраиваем опции ребенок умрет, если умрет родитель
    ProcessOptions options;
    options.kill_on_parent_exit = true;

    cout << "Запуск " << argv[1] << endl;

    if (!start_program(argv[1], g_child_process, &args, &options)) {
        cout << "не удалось запустить программу" << endl;
        if (args.argv != NULL) delete[] args.argv;
        return 1;
    }

    cout << "Родитель. Программа запущена. Ожидание." << endl;

    int code;
    if (!wait_program(g_child_process, code)) {
        cout << "не удалось получить код завершения" << endl;
        if (args.argv != NULL) delete[] args.argv;
        return 1;
    }

    cout << "код завершения дочерней программы = " << code << endl;

    if (args.argv != NULL) delete[] args.argv;
    return 0;
}