#include <windows.h>
#include <iostream>
#include <string>
#include <sstream>

using namespace std;

int main() {
    cout << "enter numbers separated by spaces: ";
    
    string input;
    getline(cin, input);
    
    if (input.empty()) {
        cout << "No input!!!!!!!!" << endl;
        return 1;
    }
    
    cout << "\nInput: " << input << endl;
    
    HANDLE hPipeMA_Read, hPipeMA_Write;
    HANDLE hPipeAP_Read, hPipeAP_Write;
    HANDLE hPipePS_Read, hPipePS_Write;
    HANDLE hPipeOut_Read, hPipeOut_Write;

    SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };
    
    if (!CreatePipe(&hPipeMA_Read, &hPipeMA_Write, &sa, 0)) {
        cerr << "Failed to create pipe M-A" << endl;
        return 1;
    }
    
    if (!CreatePipe(&hPipeAP_Read, &hPipeAP_Write, &sa, 0)) {
        cerr << "Failed to create pipe A-P" << endl;
        CloseHandle(hPipeMA_Read);
        CloseHandle(hPipeMA_Write);
        return 1;
    }
    
    if (!CreatePipe(&hPipePS_Read, &hPipePS_Write, &sa, 0)) {
        cerr << "Failed to create pipe P-S" << endl;
        CloseHandle(hPipeMA_Read);
        CloseHandle(hPipeMA_Write);
        CloseHandle(hPipeAP_Read);
        CloseHandle(hPipeAP_Write);
        return 1;
    }
    
    if (!CreatePipe(&hPipeOut_Read, &hPipeOut_Write, &sa, 0)) {
        cerr << "Failed to create pipe S-Main" << endl;
        CloseHandle(hPipeMA_Read);
        CloseHandle(hPipeMA_Write);
        CloseHandle(hPipeAP_Read);
        CloseHandle(hPipeAP_Write);
        CloseHandle(hPipePS_Read);
        CloseHandle(hPipePS_Write);
        return 1;
    }
    
    PROCESS_INFORMATION piM, piA, piP, piS;
    
    {
        STARTUPINFOA si = { sizeof(si) };
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdInput = hPipePS_Read;
        si.hStdOutput = hPipeOut_Write;
        si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
        
        if (!CreateProcessA(NULL, (LPSTR)"S.exe", NULL, NULL, TRUE, 
                           CREATE_NO_WINDOW, NULL, NULL, &si, &piS)) {
            cerr << "Failed to create process S" << endl;
            return 1;
        }
        cout << "  Process S started (PID: " << piS.dwProcessId << ")" << endl;
    }
    
    {
        STARTUPINFOA si = { sizeof(si) };
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdInput = hPipeAP_Read;
        si.hStdOutput = hPipePS_Write;
        si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
        
        if (!CreateProcessA(NULL, (LPSTR)"P.exe", NULL, NULL, TRUE, 
                           CREATE_NO_WINDOW, NULL, NULL, &si, &piP)) {
            cerr << "Failed to create process P" << endl;
            return 1;
        }
        cout << "  Process P started (PID: " << piP.dwProcessId << ")" << endl;
    }
    
    {
        STARTUPINFOA si = { sizeof(si) };
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdInput = hPipeMA_Read;
        si.hStdOutput = hPipeAP_Write;
        si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
        
        if (!CreateProcessA(NULL, (LPSTR)"A.exe", NULL, NULL, TRUE, 
                           CREATE_NO_WINDOW, NULL, NULL, &si, &piA)) {
            cerr << "Failed to create process A" << endl;
            return 1;
        }
        cout << "  Process A started (PID: " << piA.dwProcessId << ")" << endl;
    }
    
    HANDLE hInputRead, hInputWrite;
    if (!CreatePipe(&hInputRead, &hInputWrite, &sa, 0)) {
        cerr << "Failed to create input pipe" << endl;
        return 1;
    }
    
    {
        STARTUPINFOA si = { sizeof(si) };
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdInput = hInputRead;
        si.hStdOutput = hPipeMA_Write;
        si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
        
        if (!CreateProcessA(NULL, (LPSTR)"M.exe", NULL, NULL, TRUE, 
                           CREATE_NO_WINDOW, NULL, NULL, &si, &piM)) {
            cerr << "Failed to create process M" << endl;
            return 1;
        }
        cout << "  Process M started (PID: " << piM.dwProcessId << ")" << endl;
    }
    
    CloseHandle(hPipeMA_Read);
    CloseHandle(hPipeMA_Write);
    CloseHandle(hPipeAP_Read);
    CloseHandle(hPipeAP_Write);
    CloseHandle(hPipePS_Read);
    CloseHandle(hPipePS_Write);
    CloseHandle(hPipeOut_Write);
    CloseHandle(hInputRead);
    
    string data = input + "\n";
    DWORD bytesWritten;
    
    if (!WriteFile(hInputWrite, data.c_str(), (DWORD)data.length(), &bytesWritten, NULL)) {
        cerr << "Failed to write input data" << endl;
    }
    
    CloseHandle(hInputWrite);
    
    HANDLE processes[4] = { piM.hProcess, piA.hProcess, piP.hProcess, piS.hProcess };
    WaitForMultipleObjects(4, processes, TRUE, 1000);
    
    char buffer[1024];
    DWORD bytesRead;
    string result;
    
    PeekNamedPipe(hPipeOut_Read, NULL, 0, NULL, &bytesRead, NULL);
    
    if (bytesRead > 0) {
        if (ReadFile(hPipeOut_Read, buffer, sizeof(buffer) - 1, &bytesRead, NULL)) {
            buffer[bytesRead] = '\0';
            result = buffer;
            
            size_t pos = result.find_last_not_of(" \t\n\r");
            if (pos != string::npos) {
                result = result.substr(0, pos + 1);
            }
            
            cout << "result (sum): " << result << endl;
        } else {
            cerr << "Failed to read result from pipe" << endl;
        }
    } else {
        cout << "No data in output pipe (processes may have terminated)" << endl;
    }
    
    CloseHandle(hPipeOut_Read);
    CloseHandle(piM.hProcess);
    CloseHandle(piM.hThread);
    CloseHandle(piA.hProcess);
    CloseHandle(piA.hThread);
    CloseHandle(piP.hProcess);
    CloseHandle(piP.hThread);
    CloseHandle(piS.hProcess);
    CloseHandle(piS.hThread);

    DWORD processIds[] = { piM.dwProcessId, piA.dwProcessId, piP.dwProcessId, piS.dwProcessId };

    for (int i = 0; i < 4; i++) {
        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processIds[i]);
        if (hProcess) {
            TerminateProcess(hProcess, 0);
            CloseHandle(hProcess);
        }
    }

    Sleep(500);
    return 0;
}