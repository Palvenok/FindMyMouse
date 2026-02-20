#include <windows.h>

int main(int argc, char* argv[]) {
    if (argc < 2) return 1;
    DWORD pid = atoi(argv[1]);
    HANDLE hProcess = OpenProcess(SYNCHRONIZE, FALSE, pid);
    if (hProcess) {
        WaitForSingleObject(hProcess, INFINITE);
        CloseHandle(hProcess);
    }
    // Сброс курсоров
    SystemParametersInfo(SPI_SETCURSORS, 0, NULL, SPIF_SENDCHANGE);
    return 0;
}
