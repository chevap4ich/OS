#include <iostream>
#include <string>
#include <sstream>

using namespace std;

int main() {
    string line;
    
    while(getline(cin, line)) {
        stringstream ss(line);
        double number;
        double sum = 0;
        
        while (ss >> number) {
            sum += number;
        }
        
        cout << sum << endl;
    }
    
    return 0;
}
