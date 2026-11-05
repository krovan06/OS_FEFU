#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace std;

int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    cout << "Дочерняя программа: запуск" << endl;
    cout << "Получено аргументов: " << argc - 1 << endl;
    
    for (int i = 1; i < argc; ++i) {
        cout << "  argv[" << i << "] = " << argv[i] << endl;
    }

    cout << "Дочерняя программа: работа (3 сек)..." << endl;
    
#ifdef _WIN32
    Sleep(3000);
#else
    sleep(3);
#endif

    cout << "Дочерняя программа: завершение с кодом 5" << endl;
    return 5;
}
