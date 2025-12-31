#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>

using namespace std;

wstring stringToWide(const string& str) {
    if (str.empty()) return wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstr[0], size_needed);
    return wstr;
}

bool IsProcessRunning(const string& processName) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return false;
    
    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);
    
    if (!Process32FirstW(hSnapshot, &pe32)) {
        CloseHandle(hSnapshot);
        return false;
    }
    
    wstring wProcessName = stringToWide(processName);
    
    do {
        if (_wcsicmp(pe32.szExeFile, wProcessName.c_str()) == 0) {
            CloseHandle(hSnapshot);
            return true;
        }
    } while (Process32NextW(hSnapshot, &pe32));
    
    CloseHandle(hSnapshot);
    return false;
}

int main() {
    cout << "User process started" << endl;
    
    FILE* file = fopen("Killer.exe", "r");
    if (file == NULL) {
        cout << "\nERROR: Killer.exe not found in current directory!" << endl;
        cout << "Please compile it first." << endl;
        return 1;
    }
    fclose(file);
    
    cout << "\nKiller.exe found. Starting demonstration" << endl;

    cout << "\nKiller demonstration" << endl;
    
    cout << "\n1. Setting environment variable PROC_TO_KILL=notepad.exe,mspaint.exe" << endl;
    SetEnvironmentVariableA("PROC_TO_KILL", "notepad.exe,mspaint.exe");
    
    cout << "\n2. Launching test processes" << endl;
    system("start notepad.exe");
    system("start mspaint.exe"); 
    Sleep(3000);
    
    cout << "\n3. Checking running processes:" << endl;
    cout << "   Notepad.exe running: " << (IsProcessRunning("notepad.exe") ? "yes" : "no") << endl;
    cout << "   MSPaint.exe running: " << (IsProcessRunning("mspaint.exe") ? "yes" : "no") << endl;
    
    cout << "\n4. Running Killer without parameters (uses PROC_TO_KILL)" << endl;
    system("Killer.exe");
    
    cout << "\n5. After Killer:" << endl;
    cout << "   Notepad.exe running: " << (IsProcessRunning("notepad.exe") ? "yes" : "no") << endl;
    cout << "   MSPaint.exe running: " << (IsProcessRunning("mspaint.exe") ? "yes" : "no") << endl;
    
    cout << "\n6. Removing PROC_TO_KILL environment variable" << endl;
    SetEnvironmentVariableA("PROC_TO_KILL", NULL);

    cout << "\nDemonstration finished" << endl;
    return 0;
}
