# 🖱️ FindMyMouse (macOS-style for Windows)

A simple and efficient system utility built with **C (Win32 API)** that helps you instantly locate your cursor on large monitors by enlarging it when "shaken." A faithful recreation of the famous macOS feature.

![FindMyMouse Demo](assets/preview.gif)

## ✨ Features
*   **Shake to Find:** The enlargement triggers only during rapid multi-directional movements (zigzags).
*   **Smart Filtering:** The algorithm ignores fast linear mouse movements across the screen.
*   **System Tray:** Runs in the background, stays out of the taskbar, and is managed via a tray icon.
*   **Watchdog System:** A dedicated guard process (`MouseGuard`) ensures the system cursor is restored even if the main app is killed via Task Manager.
*   **Native & Lightweight:** Written in pure C, consuming only about 2-4 MB of RAM.

## 🛠️ Build & Installation

The project is developed in **CLion** (JetBrains) using **CMake**.

1.  **Prepare Resources:**
    *   Place your `icon.ico` file in the project root.
    *   Ensure the `resource.rc` file contains the line: `101 ICON "icon.ico"`.
2.  **Compilation:**
    *   Open the project in CLion.
    *   Build the `Guard` target (compiles to `MouseGuard.exe`).
    *   Build the main `FindMyMouse` target.
3.  **Run:**
    *   Launch `FindMyMouse.exe`. The app will automatically launch the guard process from the same folder.
  
You can download the pre-compiled version [here](https://github.com/Palvenok/FindMyMouse/releases).<br/>
Extract the archive to a convenient location and run `FindMyMouse.exe`.

## ⚙️ Configuration (main.c)

You can customize the utility's behavior by editing the macros at the beginning of `main.c`:


| Parameter | Description | Default Value |
| :--- | :--- | :--- |
| `MAX_SCALE` | Maximum cursor magnification scale | `4.0f` |
| `GROW_SPEED` | "Inflation" speed | `0.6f` |
| `DECAY_SPEED` | Effect fade-out speed | `0.3f` |
| `THRESHOLD` | Shake sensitivity threshold | `40` |

## 🛡️ Safety & Recovery

The application temporarily replaces the system pointer with a transparent one during the animation. 
*   If the program exits normally or via the tray icon, the cursor restores automatically.
*   If the process is "killed," `MouseGuard.exe` will restore the cursor within 1 second.
*   In case of emergency: Press `Win+R`, type `main.cpl`, go to the "Pointers" tab, and click "Use Default."

## 📜 License
MIT License. Feel free to use and modify it as you wish!
