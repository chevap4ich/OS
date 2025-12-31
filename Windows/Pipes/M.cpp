#include <iostream>
#include <string>
#include <sstream>

using namespace std;

int main() {
    string line;
    
    while (getline(cin, line)) {
        stringstream ss(line);
        double number;
        bool first = true;
        
        while (ss >> number) {
            if (!first) {
                cout << " ";
            }
            cout << number * 7;
            first = false;
        }
        cout << endl;
    }
    
    return 0;
}