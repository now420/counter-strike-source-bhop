#include <iostream>
#include <windows.h>
#include <TlHelp32.h>

bool IsGameInFocus(const char* windowTitle) {
    HWND gameWindow = FindWindow(NULL, windowTitle);
    return GetForegroundWindow() == gameWindow;
}

DWORD GetModuleBaseAddress(DWORD processID, const char* moduleName) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processID);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    MODULEENTRY32 moduleEntry;
    moduleEntry.dwSize = sizeof(MODULEENTRY32);

    if (Module32First(hSnapshot, &moduleEntry)) {
        do {
            if (_stricmp(moduleEntry.szModule, moduleName) == 0) {
                CloseHandle(hSnapshot);
                return (DWORD)moduleEntry.modBaseAddr;
            }
        } while (Module32Next(hSnapshot, &moduleEntry));
    }

    CloseHandle(hSnapshot);
    return 0;
}

int main() {
    HWND window = FindWindow(NULL, "Counter-Strike Source");
    if (!window) {
        std::cerr << "open counterstrike stupid monkey" << std::endl;
        Sleep(2500);
        return 1;
    }

    DWORD pid;
    GetWindowThreadProcessId(window, &pid);

    HANDLE hproc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hproc) {
        std::cerr << "openprocess failed" << std::endl;
        Sleep(2500);
        return 1;
    }

    DWORD clientBaseAddress = GetModuleBaseAddress(pid, "client.dll");
    if (!clientBaseAddress) {
        std::cerr << "failed to find client.dll :(" << std::endl;
        CloseHandle(hproc);
        Sleep(2500);
        return 1;
    }

    DWORD jumpOffset = 0x4F5D24;
    DWORD airOffset = 0x4A4078;
    DWORD jumpAddress = clientBaseAddress + jumpOffset;
    DWORD airAddress = clientBaseAddress + airOffset;

    std::cout << "Client.dll Base Address: " << std::hex << clientBaseAddress << std::endl;
    std::cout << "Jump Address: " << std::hex << jumpAddress << std::endl;
    std::cout << "Air Address: " << std::hex << airAddress << std::endl;

    while (true) {
        if (GetAsyncKeyState(VK_END) & 1) {
            break;
        }

        if (IsGameInFocus("Counter-Strike Source") && GetAsyncKeyState(VK_SPACE)) {
            int airFlag;
            if (ReadProcessMemory(hproc, (LPCVOID)airAddress, &airFlag, sizeof(airFlag), NULL)) {
                if (airFlag == 0) {
                    int jumpValue = 5;
                    WriteProcessMemory(hproc, (LPVOID)jumpAddress, &jumpValue, sizeof(jumpValue), NULL);
                    Sleep(1); // you can change this value for better bhop (u cant remove the sleep entirely or jumping wont work, i tried it)
                    jumpValue = 4;
                    WriteProcessMemory(hproc, (LPVOID)jumpAddress, &jumpValue, sizeof(jumpValue), NULL);
                }
            }
        }

        Sleep(1);
        // ^ u can uncomment this for **slightly** better bhop although theres no visible difference besides the amount of resources being used
    }

    CloseHandle(hproc);
    std::cout << "Exiting..." << std::endl;
    return 0;
}
