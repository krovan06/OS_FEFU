// Настройки поведения процесса
#ifndef PROCESS_H
#define PROCESS_H

#include <cstddef>

#ifdef _WIN32
#include <windows.h>
typedef HANDLE Process;
#else
#include <sys/types.h>
typedef pid_t Process;
#endif

// Структура для передачи аргументов командной строки
struct ProcessArgs {
    const char** argv;
    int count;
    ProcessArgs() : argv(NULL), count(0) {}
};

struct ProcessOptions {
    bool kill_on_parent_exit; // Если true, ребенок умрет вместе с родителем
    ProcessOptions() : kill_on_parent_exit(false) {}
};

bool start_program(const char* path, Process& process, 
                   const ProcessArgs* args = NULL, 
                   const ProcessOptions* options = NULL);

bool wait_program(Process process, int& exit_code);
bool terminate_program(Process process);

#endif // PROCESS_H
