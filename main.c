#include <windows.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define IDI_MYICON 101

#define MAX_SCALE 4.0f
#define MIN_SCALE 1.0f
#define GROW_SPEED 0.6f
#define DECAY_SPEED 0.3f
#define THRESHOLD 40

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_EXIT 2001
#define ID_TRAY_REFRESH 2002

NOTIFYICONDATAA g_nid = {0};

float g_currentScale = 1.0f;
POINT g_mousePos = {0, 0};
BOOL g_isCursorHidden = FALSE;
HCURSOR g_hOriginalCursor = NULL;

const DWORD g_cursorIds[] = {
    32512, // IDC_ARROW
    32513, // IDC_IBEAM (Текст)
    32514, // IDC_WAIT (Песочные часы)
    32515, // IDC_CROSS (Крестик)
    32516, // IDC_UPARROW
    32642, // IDC_SIZENWSE
    32643, // IDC_SIZENESW
    32644, // IDC_SIZEWE
    32645, // IDC_SIZENS
    32646, // IDC_SIZEALL
    32648, // IDC_NO
    32649, // IDC_HAND (Рука/Ссылка)
    32650, // IDC_APPSTARTING (Фоновая загрузка)
    32651  // IDC_HELP
};


// Безопасное восстановление курсора
void RestoreSystemCursor() {
    if (g_isCursorHidden) {
        SystemParametersInfo(SPI_SETCURSORS, 0, NULL, SPIF_SENDCHANGE);
        g_isCursorHidden = FALSE;
    }
}

// Создание гарантированно пустого курсора
HCURSOR CreateEmptyCursor() {
    int cw = GetSystemMetrics(SM_CXCURSOR);
    int ch = GetSystemMetrics(SM_CYCURSOR);

    // Создаем пустую маску: AND = все 1 (прозрачно), XOR = все 0
    BYTE* andMask = (BYTE*)malloc(cw * ch / 8);
    BYTE* xorMask = (BYTE*)malloc(cw * ch / 8);
    if (andMask) memset(andMask, 0xFF, cw * ch / 8);
    if (xorMask) memset(xorMask, 0x00, cw * ch / 8);

    HCURSOR hEmpty = CreateCursor(GetModuleHandle(NULL), 0, 0, cw, ch, andMask, xorMask);

    free(andMask);
    free(xorMask);
    return hEmpty;
}

void RefreshOriginalCursor() {
    // Если старая копия была, удаляем её из памяти
    if (g_hOriginalCursor) {
        DestroyCursor(g_hOriginalCursor);
    }
    // Загружаем актуальный системный курсор и делаем свежую копию
    HCURSOR hSystemArrow = LoadCursor(NULL, IDC_ARROW);
    g_hOriginalCursor = CopyCursor(hSystemArrow);
}


void ToggleSystemCursor(BOOL hide) {
    if (hide && !g_isCursorHidden) {
        int count = sizeof(g_cursorIds) / sizeof(g_cursorIds[0]);
        for (int i = 0; i < count; i++) {
            HCURSOR hEmpty = CreateEmptyCursor();
            if (hEmpty) {
                SetSystemCursor(hEmpty, g_cursorIds[i]);
            }
        }
        g_isCursorHidden = TRUE;
    } else if (!hide && g_isCursorHidden) {
        RestoreSystemCursor();
    }
}

// Функция для добавления иконки в трей
void AddTrayIcon(HWND hwnd) {
    g_nid.cbSize = sizeof(NOTIFYICONDATAA);
    g_nid.hWnd = hwnd;
    g_nid.uID = 1;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_TRAYICON;

    // Загружаем иконку из ресурсов нашего модуля
    g_nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_MYICON));

    strcpy(g_nid.szTip, "FindMyMouse");
    Shell_NotifyIconA(NIM_ADD, &g_nid);
}


// Функция для удаления иконки
void RemoveTrayIcon() {
    Shell_NotifyIconA(NIM_DELETE, &g_nid);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_TRAYICON:
            if (lParam == WM_RBUTTONUP) {
                POINT curPoint;
                GetCursorPos(&curPoint);
                HMENU hMenu = CreatePopupMenu();
                // Добавляем новый пункт
                InsertMenu(hMenu, 0, MF_BYPOSITION | MF_STRING, ID_TRAY_REFRESH, "Refresh Cursor");
                InsertMenu(hMenu, 1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
                InsertMenu(hMenu, 2, MF_BYPOSITION | MF_STRING, ID_TRAY_EXIT, "Exit");

                SetForegroundWindow(hwnd);
                TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, curPoint.x, curPoint.y, 0, hwnd, NULL);
                DestroyMenu(hMenu);
            }
            break;

        case WM_COMMAND:
            if (LOWORD(wParam) == ID_TRAY_REFRESH) {
                RefreshOriginalCursor();
                // Опционально: сбрасываем системные курсоры на случай багов
                RestoreSystemCursor();
            }
            else if (LOWORD(wParam) == ID_TRAY_EXIT) {
                DestroyWindow(hwnd);
            }
            break;

        case WM_PAINT:
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            if (g_currentScale > 1.1f && g_hOriginalCursor != NULL) {
                ICONINFO ii;
                if (GetIconInfo(g_hOriginalCursor, &ii)) {
                    int baseSize = GetSystemMetrics(SM_CXCURSOR);
                    int newSize = (int)(baseSize * g_currentScale);
                    int offX = (int)(ii.xHotspot * g_currentScale);
                    int offY = (int)(ii.yHotspot * g_currentScale);

                    DrawIconEx(hdc, g_mousePos.x - offX, g_mousePos.y - offY,
                               g_hOriginalCursor, newSize, newSize, 0, NULL, DI_NORMAL);

                    if (ii.hbmMask) DeleteObject(ii.hbmMask);
                    if (ii.hbmColor) DeleteObject(ii.hbmColor);
                }
            }
            EndPaint(hwnd, &ps);
            break;

        case WM_DESTROY:
            RemoveTrayIcon();
            RestoreSystemCursor();
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR lpCmd, int nShow) {
    // Проверка на единственную копию
    HANDLE hMutex = CreateMutexA(NULL, TRUE, "Global\\MouseFinderMutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) return 0;

    // Запуск стража (Guard.exe должен быть в той же папке)
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    char* lastSlash = strrchr(path, '\\');
    if (lastSlash) *(lastSlash + 1) = '\0';
    strcat(path, "MouseGuard.exe");

    char args[64];
    sprintf(args, "%lu", GetCurrentProcessId());

    // Запускаем скрыто
    ShellExecuteA(NULL, "open", path, args, NULL, SW_HIDE);

    atexit(RestoreSystemCursor);

    // Копируем оригинал до всех манипуляций
    // g_hOriginalCursor = CopyCursor(LoadCursor(NULL, IDC_ARROW));
    // Вместо ручного CopyCursor в начале WinMain:
    RefreshOriginalCursor();

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "MacMouseOverlay";
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_MYICON));
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW,
        "MacMouseOverlay", NULL, WS_POPUP, 0, 0,
        GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
        NULL, NULL, hInstance, NULL);

    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
    ShowWindow(hwnd, SW_SHOW);
    AddTrayIcon(hwnd);

    int shakeCount = 0;      // Счетчик резких смен направления
    float lastDx = 0;        // Предыдущая скорость по X
    float lastDy = 0;        // Предыдущая скорость по Y
    DWORD lastShakeTime = 0; // Время последнего "разворота"
    POINT lastPos;
    GetCursorPos(&lastPos);

    MSG msg = {0};
    while (1) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) goto end;
            DispatchMessage(&msg);
        }

        POINT currentPos;
        GetCursorPos(&currentPos);

        float dx = (float)(currentPos.x - lastPos.x);
        float dy = (float)(currentPos.y - lastPos.y);

        // 1. Проверяем резкую смену направления (разворот по X или Y)
        // Если знак скорости изменился и скорость была приличной (> 10 пикселей)
        if (((dx > 5 && lastDx < -5) || (dx < -5 && lastDx > 5) ||
             (dy > 5 && lastDy < -5) || (dy < -5 && lastDy > 5))) {

            shakeCount++;
            lastShakeTime = GetTickCount(); // Запоминаем время рывка
             }

        // 2. Если давно не трясли (более 400 мс), сбрасываем счетчик рывков
        if (GetTickCount() - lastShakeTime > 400) {
            shakeCount = 0;
        }

        // 3. Активация: нужно сделать минимум 3-4 резких разворота
        if (shakeCount >= 3) {
            g_currentScale += GROW_SPEED;
        } else {
            g_currentScale -= DECAY_SPEED;
        }

        // Ограничители масштаба
        if (g_currentScale > MAX_SCALE) g_currentScale = MAX_SCALE;
        if (g_currentScale < MIN_SCALE) g_currentScale = MIN_SCALE;

        // 4. Отрисовка
        if (g_currentScale > 1.15f) {
            if (!g_isCursorHidden) ToggleSystemCursor(TRUE); // Скрываем только один раз
            g_mousePos = currentPos;
            InvalidateRect(hwnd, NULL, TRUE);
        } else {
            if (g_isCursorHidden) ToggleSystemCursor(FALSE); // Возвращаем только один раз
            InvalidateRect(hwnd, NULL, TRUE);
        }


        if (dx != 0) lastDx = dx;
        if (dy != 0) lastDy = dy;
        lastPos = currentPos;

        //if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) break;
        Sleep(16);
    }

end:
    ReleaseMutex(hMutex);
    CloseHandle(hMutex);
    RestoreSystemCursor();
    return 0;
}
