#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <sys/types.h>
#include <fstream>

using namespace std;

bool ProcessExists(pid_t pid) {
    return kill(pid, 0) == 0;
}

bool TerminateProcessByID(pid_t processId) {
    if (!ProcessExists(processId)) {
        cerr << "Process with ID " << processId << " does not exist." << endl;
        return false;
    }
    
    if (kill(processId, SIGTERM) == 0) {
        cout << "Process with ID " << processId << " terminated successfully." << endl;
        return true;
    } else {
        cerr << "Failed to terminate process with ID " << processId << endl;
        return false;
    }
}

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

bool TerminateProcessByName(const string& processName) {
    bool found = false;
    
    DIR* dir = opendir("/proc");
    if (!dir) {
        cerr << "Cannot open /proc directory" << endl;
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
            cout << "Found process: " << name << " (ID: " << pid << ")" << endl;
            if (TerminateProcessByID(pid)) found = true;
        }
    }
    
    closedir(dir);
    
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

string TrimString(const string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == string::npos) return "";
    
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

void TerminateFromEnvironment() {
    const char* procList = getenv("PROC_TO_KILL");
    
    if (procList == nullptr || strlen(procList) == 0) {
        cout << "Environment variable PROC_TO_KILL is not set." << endl;
        return;
    }
    
    cout << "PROC_TO_KILL = " << procList << endl;
    
    vector<string> processes = SplitString(procList, ',');
    
    for (const auto& procName : processes) {
        string trimmedName = TrimString(procName);
        
        if (!trimmedName.empty()) {
            cout << "Terminating processes with name: " << trimmedName << endl;
            TerminateProcessByName(trimmedName);
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
            pid_t pid = atoi(argv[i + 1]);
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