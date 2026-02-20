cmake_minimum_required(VERSION 4.0)
project(FindMyMouse C)

set(CMAKE_C_STANDARD 23)

# Добавляем флаг -mwindows, чтобы приложение работало в фоне без консоли
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows")

add_executable(FindMyMouse main.c resource.rc)

enable_language(RC)

# Подключаем системные библиотеки Windows
target_link_libraries(FindMyMouse user32 gdi32 shell32)

# Страж (отдельный exe)
add_executable(MouseGuard guard.c)
target_link_libraries(MouseGuard user32)
