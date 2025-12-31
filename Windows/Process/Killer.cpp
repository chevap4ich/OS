#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <string>
#include <vector>
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

string wideToString(const wstring& wstr) {
    if (wstr.empty()) return string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    string str(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &str[0], size_needed, NULL, NULL);
    return str;
}

bool TerminateProcessByID(DWORD processId) {
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processId);
    if (hProcess == NULL) {
        cerr << "Failed to open process with ID " << processId << endl;
        return false;
    }
    
    bool result = TerminateProcess(hProcess, 0);
    if (!result) {
        cerr << "Failed to terminate process with ID " << processId << endl;
    } else {
        cout << "Process with ID " << processId << " terminated successfully." << endl;
    }
    
    CloseHandle(hProcess);
    return result;
}

bool TerminateProcessByName(const string& processName) {
    bool found = false;
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
            cout << "Found process: " << wideToString(pe32.szExeFile) 
                 << " (ID: " << pe32.th32ProcessID << ")" << endl;
            if (TerminateProcessByID(pe32.th32ProcessID)) found = true;
        }
    } while (Process32NextW(hSnapshot, &pe32));
    
    CloseHandle(hSnapshot);
    
    if (!found) {
        cout << "No processes with name '" << processName << "' found." << endl;
    }
    
    return found;
}

vector<string> SplitString(const string& str, char delimiter) {
    vector<string> result;
    size_t start = 0;
    size_t end = str.find(delimiter);
    
    while (end != string::npos) {
        result.push_back(str.substr(start, end - start));
        start = end + 1;
        end = str.find(delimiter, start);
    }
    
    result.push_back(str.substr(start));
    return result;
}

void TerminateFromEnvironment() {
    char buffer[4096];
    DWORD length = GetEnvironmentVariableA("PROC_TO_KILL", buffer, sizeof(buffer));
    
    if (length == 0) {
        cout << "Environment variable PROC_TO_KILL is not set." << endl;
        return;
    }
    
    if (length >= sizeof(buffer)) {
        cerr << "PROC_TO_KILL variable is too long." << endl;
        return;
    }
    
    string procList = buffer;
    cout << "PROC_TO_KILL = " << procList << endl;
    
    vector<string> processes = SplitString(procList, ',');
    
    for (const auto& procName : processes) {
        string trimmedName = procName;
        size_t start = trimmedName.find_first_not_of(" \t");
        size_t end = trimmedName.find_last_not_of(" \t");
        
        if (start != string::npos && end != string::npos) {
            trimmedName = trimmedName.substr(start, end - start + 1);
            
            if (!trimmedName.empty()) {
                cout << "Terminating processes with name: " << trimmedName << endl;
                TerminateProcessByName(trimmedName);
            }
        }
    }
}

void PrintHelp() {
    cout << "Usage: Killer [options]" << endl;
    cout << "Options:" << endl;
    cout << "  --id <PID>      : Terminate process by specified ID" << endl;
    cout << "  --name <name>   : Terminate all processes with specified name" << endl;
    cout << "  --help          : Show this help message" << endl;
    cout << endl;
    cout << "Also terminates processes from PROC_TO_KILL environment variable" << endl;
}

int main(int argc, char* argv[]) {
    cout << "Killer process started" << endl;
    
    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        
        if (arg == "--id" && i + 1 < argc) {
            DWORD pid = atoi(argv[i + 1]);
            cout << "Terminating process by ID: " << pid << endl;
            TerminateProcessByID(pid);
            i++;
        }
        else if (arg == "--name" && i + 1 < argc) {
            string name = argv[i + 1];
            cout << "Terminating processes with name: " << name << endl;
            TerminateProcessByName(name);
            i++;
        }
        else if (arg == "--help") {
            PrintHelp();
            return 0;
        }
        else {
            cerr << "Unknown parameter: " << arg << endl;
            cerr << "Use --help for usage information." << endl;
            return 1;
        }
    }
    
    cout << endl << "Checking PROC_TO_KILL environment variable" << endl;
    TerminateFromEnvironment();
    
    cout << endl << "Killer process finished" << endl;
    return 0;
}