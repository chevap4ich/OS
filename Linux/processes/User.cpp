#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fstream>
#include <vector>
#include <dirent.h>
#include <cctype>
#include <cstdio>

using namespace std;

string GetProcessName(pid_t pid) {
    string procPath = "/proc/" + to_string(pid) + "/comm";
    ifstream file(procPath);
    if (file.is_open()) {
        string name;
        getline(file, name);
        return name;
    }
    return "";
}

bool IsProcessRunning(const string& processName) {
    DIR* dir = opendir("/proc");
    if (!dir) {
        return false;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        bool isPid = true;
        for (int i = 0; entry->d_name[i] != '\0'; i++) {
            if (!isdigit(entry->d_name[i])) {
                isPid = false;
                break;
            }
        }
        
        if (!isPid) continue;
        
        pid_t pid = atoi(entry->d_name);
        string name = GetProcessName(pid);
        
        if (!name.empty() && name == processName) {
            closedir(dir);
            return true;
        }
    }
    
    closedir(dir);
    return false;
}

int main() {
    cout << "User process started" << endl;
    
    if (access("Killer", F_OK) != 0) {
        cout << "\nERROR: Killer executable not found in current directory!" << endl;
        cout << "Please compile it first: g++ Killer.cpp -o Killer" << endl;
        return 1;
    }
    
    cout << "\nKiller executable found. Starting demonstration" << endl;
    cout << "\nKiller demonstration" << endl;
    
    cout << "\n1. Setting environment variable PROC_TO_KILL=xterm" << endl;
    setenv("PROC_TO_KILL", "xterm", 1);
    
    cout << "\n2. Launching xterm process (terminal emulator)" << endl;
    cout << "   Command: xterm -hold -e 'echo Test terminal; sleep 10' &" << endl;
    
    system("xterm -hold -e 'echo Test terminal; sleep 10' &");
    
    sleep(3);
    
    cout << "\n3. Checking running processes:" << endl;
    cout << "   xterm running: " << (IsProcessRunning("xterm") ? "yes" : "no") << endl;
    
    cout << "\n4. Running Killer without parameters (uses PROC_TO_KILL)" << endl;
    system("./Killer");
    
    sleep(3);
    
    cout << "\n5. After Killer:" << endl;
    cout << "   xterm running: " << (IsProcessRunning("xterm") ? "yes" : "no") << endl;
    
    cout << "\n6. Removing PROC_TO_KILL environment variable" << endl;
    unsetenv("PROC_TO_KILL");
    
    cout << "\n7. Testing Killer with --name parameter" << endl;
    
    cout << "\n   Launching xterm again for testing..." << endl;
    system("xterm -hold -e 'echo Test 2; sleep 10' &");
    sleep(3);
    
    cout << "   xterm running: " << (IsProcessRunning("xterm") ? "yes" : "no") << endl;
    
    cout << "   Testing: ./Killer --name xterm" << endl;
    system("./Killer --name xterm");
    
    sleep(3);
    cout << "   xterm running after --name: " << (IsProcessRunning("xterm") ? "yes" : "no") << endl;
    
    cout << "\n8. Testing Killer with --id parameter" << endl;
    
    cout << "\n   Launching xterm to get its PID..." << endl;
    
    pid_t xtermPid = 0;
    
    string cmd = "xterm -hold -e 'echo Test PID; sleep 30' & echo $!";
    FILE* pipe = popen(cmd.c_str(), "r");
    if (pipe) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            xtermPid = atoi(buffer);
        }
        pclose(pipe);
    }
    
    sleep(3);
    
    cout << "   xterm PID: " << xtermPid << endl;
    cout << "   xterm running: " << (IsProcessRunning("xterm") ? "yes" : "no") << endl;
    
    cout << "   Testing: ./Killer --id " << xtermPid << endl;
    
    string killCmd = "./Killer --id " + to_string(xtermPid);
    system(killCmd.c_str());
    
    sleep(3);
    cout << "   xterm running after --id: " << (IsProcessRunning("xterm") ? "yes" : "no") << endl;

    cout << "\n9. Final cleanup - killing all xterm processes" << endl;
    system("pkill -9 xterm 2>/dev/null");
    sleep(2);
    cout << "   Final check - xterm running: " << (IsProcessRunning("xterm") ? "yes" : "no") << endl;
    
    cout << "\nDemonstration finished" << endl;
    return 0;
}