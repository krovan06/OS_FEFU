#include "process.h"
#include <cstddef>

#ifdef _WIN32
#include <cstdlib>

bool start_program(const char* path, Process& process, const ProcessArgs* args, const ProcessOptions* options) {
    if (path == NULL || path[0] == '\0') return false;

    STARTUPINFOA startup = {0};
    startup.cb = sizeof(startup);
    PROCESS_INFORMATION info = {0};

    // Формируем командную строку из аргументов
    char* cmd_line = NULL;
    if (args != NULL && args->argv != NULL && args->count > 0) {
        size_t total_len = 0;
        for (int i = 0; i < args->count; ++i) {
            if (args->argv[i] != NULL) total_len += strlen(args->argv[i]) + 1;
        }
        if (total_len > 0) {
            cmd_line = new char[total_len + 1];
            size_t pos = 0;
            for (int i = 0; i < args->count; ++i) {
                if (args->argv[i] != NULL) {
                    size_t arg_len = strlen(args->argv[i]);
                    if (pos > 0) cmd_line[pos++] = ' ';
                    strcpy(cmd_line + pos, args->argv[i]);
                    pos += arg_len;
                }
            }
            cmd_line[pos] = '\0';
        }
    }

    BOOL result = CreateProcessA(path, cmd_line, NULL, NULL, FALSE, 0, NULL, NULL, &startup, &info);
    if (cmd_line != NULL) delete[] cmd_line;
    if (!result) return false;

    process = info.hProcess;
    CloseHandle(info.hThread);
    return true;
}

bool wait_program(Process process, int& exit_code) {
    if (process == NULL || process == INVALID_HANDLE_VALUE) return false;
    if (WaitForSingleObject(process, INFINITE) != WAIT_OBJECT_0) {
        CloseHandle(process);
        return false;
    }
    DWORD code;
    BOOL success = GetExitCodeProcess(process, &code);
    CloseHandle(process);
    if (!success) return false;
    exit_code = static_cast<int>(code);
    return true;
}

bool terminate_program(Process process) {
    if (process == NULL || process == INVALID_HANDLE_VALUE) return false;
    BOOL result = TerminateProcess(process, 1);
    CloseHandle(process);
    return result != FALSE;
}

#else // POSIX (Linux / macOS)
#include <unistd.h>
#include <sys/wait.h>
#include <sys/prctl.h> // Для PR_SET_PDEATHSIG
#include <csignal>
#include <cerrno>

bool start_program(const char* path, Process& process, const ProcessArgs* args, const ProcessOptions* options) {
    if (path == NULL || path[0] == '\0') return false;

    pid_t pid = fork();
    if (pid < 0) return false; // Ошибка fork

    if (pid == 0) {
        // --- МЫ В ДОЧЕРНЕМ ПРОЦЕССЕ ---
        
        // РЕШЕНИЕ ЗАМЕЧАНИЯ ПРЕПОДАВАТЕЛЯ: поведение при смерти родителя
        if (options != NULL && options->kill_on_parent_exit) {
            prctl(PR_SET_PDEATHSIG, SIGTERM);
            // Дополнительная проверка: если родитель уже умер до вызова prctl
            if (getppid() == 1) {
                _exit(1);
            }
        }

        // Формируем массив аргументов для execvp
        char** child_argv = NULL;
        if (args != NULL && args->argv != NULL && args->count > 0) {
            child_argv = new char*[args->count + 2];
            child_argv[0] = const_cast<char*>(path);
            for (int i = 0; i < args->count; ++i) {
                child_argv[i + 1] = const_cast<char*>(args->argv[i]);
            }
            child_argv[args->count + 1] = NULL;
        } else {
            child_argv = new char*[2];
            child_argv[0] = const_cast<char*>(path);
            child_argv[1] = NULL;
        }

        // Заменяем образ процесса на новую программу
        execvp(path, child_argv);
        
        // Если execvp вернул управление, значит произошла ошибка
        delete[] child_argv;
        _exit(127);
    }

    // --- МЫ В РОДИТЕЛЬСКОМ ПРОЦЕССЕ ---
    process = pid;
    return true;
}

bool wait_program(Process process, int& exit_code) {
    if (process <= 0) return false;

    int status;
    // Ждем завершения конкретного PID. Без кривых while, просто честный waitpid
    pid_t result = waitpid(process, &status, 0);
    
    if (result == -1) return false;

    if (WIFEXITED(status)) {
        exit_code = WEXITSTATUS(status);
        return true;
    }
    if (WIFSIGNALED(status)) {
        // Если процесс убит сигналом, возвращаем 128 + номер сигнала (стандарт shell)
        exit_code = 128 + WTERMSIG(status);
        return true;
    }
    return false;
}

bool terminate_program(Process process) {
    if (process <= 0) return false;
    int result = kill(process, SIGTERM);
    if (result == 0) {
        int status;
        waitpid(process, &status, 0); // Ждем, пока он действительно умрет
    }
    return result == 0;
}
#endif
