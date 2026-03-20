#include <windows.h>
#include <wininet.h>
#include <psapi.h>
#include <iostream>
#include <string>
#include <filesystem>
#include <thread>
#include <tlhelp32.h>
#include <vector>
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "psapi.lib")

namespace fs = std::filesystem;

const char* URL = "https://github.com/zetsr/Begeerte-ASA/raw/refs/heads/main/release/begeerte_ark_survival_ascended.dll";
const char* DLL_NAME = "begeerte_ark_survival_ascended.dll";
const char* TARGET_PROCESS = "ArkAscended.exe";

// --- 工具函数：控制台输出 ---
void WriteToConsoleBuffer(const std::string& text) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD written = 0;
    WriteFile(h, text.c_str(), (DWORD)text.length(), &written, NULL);
}

// --- 业务逻辑：下载 DLL ---
bool DownloadDLL() {
    const char* TEMP_DLL_NAME = "begeerte_ark_survival_ascended.dll.tmp";
    HINTERNET hInternet = InternetOpenA("DLLDownloader/1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hInternet) return false;

    HINTERNET hUrl = InternetOpenUrlA(hInternet, URL, "User-Agent: Mozilla/5.0\r\n", 0, INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_RELOAD, 0);
    if (!hUrl) {
        InternetCloseHandle(hInternet);
        return false;
    }

    DWORD fileSize = 0;
    DWORD sizeLen = sizeof(DWORD);
    HttpQueryInfoA(hUrl, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &fileSize, &sizeLen, NULL);

    HANDLE hFile = CreateFileA(TEMP_DLL_NAME, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        InternetCloseHandle(hUrl);
        InternetCloseHandle(hInternet);
        return false;
    }

    const DWORD BUFFER_SIZE = 1048576;
    BYTE* buffer = (BYTE*)malloc(BUFFER_SIZE);
    DWORD downloaded = 0;
    bool success = true;

    while (true) {
        DWORD bytesRead = 0;
        if (!InternetReadFile(hUrl, buffer, BUFFER_SIZE, &bytesRead) || bytesRead == 0) break;
        DWORD bytesWritten = 0;
        if (!WriteFile(hFile, buffer, bytesRead, &bytesWritten, NULL)) {
            success = false;
            break;
        }
        downloaded += bytesRead;
    }

    free(buffer);
    CloseHandle(hFile);
    InternetCloseHandle(hUrl);
    InternetCloseHandle(hInternet);

    if (success) {
        MoveFileExA(TEMP_DLL_NAME, DLL_NAME, MOVEFILE_REPLACE_EXISTING);
        return true;
    }
    return false;
}

// --- 业务逻辑：查找 PID ---
DWORD FindProcessId(const char* procName) {
    DWORD processID = 0;
    while (processID == 0) {
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot != INVALID_HANDLE_VALUE) {
            PROCESSENTRY32 pe32;
            pe32.dwSize = sizeof(PROCESSENTRY32);
            if (Process32First(hSnapshot, &pe32)) {
                do {
                    if (_stricmp(pe32.szExeFile, procName) == 0) {
                        processID = pe32.th32ProcessID;
                        break;
                    }
                } while (Process32Next(hSnapshot, &pe32));
            }
            CloseHandle(hSnapshot);
        }
        if (processID == 0) {
            WriteToConsoleBuffer("Waiting for " + std::string(procName) + "...\n");
            Sleep(1000);
        }
    }
    return processID;
}

// --- 业务逻辑：注入 DLL ---
bool InjectDLL(DWORD procID) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procID);
    if (!hProcess) return false;

    char dllPath[MAX_PATH];
    GetFullPathNameA(DLL_NAME, MAX_PATH, dllPath, NULL);

    LPVOID pDllPath = VirtualAllocEx(hProcess, NULL, strlen(dllPath) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    WriteProcessMemory(hProcess, pDllPath, dllPath, strlen(dllPath) + 1, NULL);

    LPVOID pLoadLibrary = GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLibrary, pDllPath, 0, NULL);

    if (hThread) {
        WaitForSingleObject(hThread, INFINITE);
        CloseHandle(hThread);
    }

    VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
    CloseHandle(hProcess);
    return hThread != NULL;
}

// --- 主程序 ---
int main() {
    WriteToConsoleBuffer(">>> Initializing Downloader...\n");
    if (!DownloadDLL()) {
        WriteToConsoleBuffer("[-] Download failed.\n");
        return 1;
    }

    DWORD procID = FindProcessId(TARGET_PROCESS);
    WriteToConsoleBuffer("[+] Target found! PID: " + std::to_string(procID) + "\n");

    WriteToConsoleBuffer("[*] Injecting DLL...\n");
    if (InjectDLL(procID)) {
        WriteToConsoleBuffer("[+] Injection successful!\n");
    }
    else {
        WriteToConsoleBuffer("[-] Injection failed!\n");
    }

    WriteToConsoleBuffer(">>> Done. Exiting in 5s...\n");
    Sleep(5000);
    return 0;
}