#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;

int main() {
    cout << "Enter numbers: ";
    string input;
    getline(cin, input);
    
    if (input.empty()) return 1;
    
    int m_a[2], a_p[2], p_s[2], s_main[2];
    pipe(m_a); pipe(a_p); pipe(p_s); pipe(s_main);
    
    // Процесс S
    if (fork() == 0) {
        close(m_a[0]); close(m_a[1]);
        close(a_p[0]); close(a_p[1]);
        close(p_s[1]); close(s_main[0]);
        
        dup2(p_s[0], STDIN_FILENO);
        dup2(s_main[1], STDOUT_FILENO);
        
        close(p_s[0]);
        close(s_main[1]);
        
        execl("./S", "S", NULL);
        exit(1);
    }
    
    // Процесс P
    if (fork() == 0) {
        close(m_a[0]); close(m_a[1]);
        close(a_p[1]); close(p_s[0]);
        close(s_main[0]); close(s_main[1]);
        
        dup2(a_p[0], STDIN_FILENO);
        dup2(p_s[1], STDOUT_FILENO);
        
        close(a_p[0]);
        close(p_s[1]);
        
        execl("./P", "P", NULL);
        exit(1);
    }
    
    // Процесс A
    if (fork() == 0) {
        close(m_a[1]); close(a_p[0]);
        close(p_s[0]); close(p_s[1]);
        close(s_main[0]); close(s_main[1]);
        
        dup2(m_a[0], STDIN_FILENO);
        dup2(a_p[1], STDOUT_FILENO);
        
        close(m_a[0]);
        close(a_p[1]);
        
        execl("./A", "A", NULL);
        exit(1);
    }
    
    // Процесс M
    if (fork() == 0) {
        // Пайп для ввода
        int input_pipe[2];
        pipe(input_pipe);
        
        // Процесс для записи входных данных
        if (fork() == 0) {
            close(input_pipe[0]);
            string data = input + "\n";
            write(input_pipe[1], data.c_str(), data.length());
            close(input_pipe[1]);
            exit(0);
        }
        
        close(input_pipe[1]);
        
        // Закрываем ненужные пайпы
        close(m_a[0]); close(a_p[0]); close(a_p[1]);
        close(p_s[0]); close(p_s[1]);
        close(s_main[0]); close(s_main[1]);
        
        dup2(input_pipe[0], STDIN_FILENO);
        dup2(m_a[1], STDOUT_FILENO);
        
        close(input_pipe[0]);
        close(m_a[1]);
        
        execl("./M", "M", NULL);
        exit(1);
    }
    
    // Родитель (Main)
    close(m_a[0]); close(m_a[1]);
    close(a_p[0]); close(a_p[1]);
    close(p_s[0]); close(p_s[1]);
    close(s_main[1]);  // Закрываем запись в s_main
    
    // Ждем все процессы
    for (int i = 0; i < 4; i++) wait(NULL);
    
    // Читаем результат
    char result[100];
    int bytes = read(s_main[0], result, sizeof(result)-1);
    
    if (bytes > 0) {
        result[bytes] = '\0';
        // Убираем \n в конце если есть
        if (result[bytes-1] == '\n') result[bytes-1] = '\0';
        cout << "Result: " << result << endl;
    } else {
        cout << "No result" << endl;
    }
    
    close(s_main[0]);
    return 0;
}