#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <random>
#include <mutex>
#include <iomanip>

class Matrix {
private:
    std::vector<std::vector<int>> data;
    int rows;
    int cols;

public:
    Matrix(int r, int c) : rows(r), cols(c) {
        data.resize(rows, std::vector<int>(cols, 0));
    }

    void randomFill(int minVal = 0, int maxVal = 10) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(minVal, maxVal);
        
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                data[i][j] = dist(gen);
            }
        }
    }

    int get(int i, int j) const { return data[i][j]; }
    void set(int i, int j, int value) { data[i][j] = value; }
    
    int getRows() const { return rows; }
    int getCols() const { return cols; }

    void print() const {
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                std::cout << std::setw(4) << data[i][j];
            }
            std::cout << std::endl;
        }
    }
};

class BlockMultiplier {
private:
    const Matrix& A;
    const Matrix& B;
    Matrix& C;
    int blockSize;
    std::mutex mtx;

public:
    BlockMultiplier(const Matrix& a, const Matrix& b, Matrix& c, int k) 
        : A(a), B(b), C(c), blockSize(k) {}

    void multiplyBlock(int rowBlock, int colBlock) {
        int startRow = rowBlock * blockSize;
        int startCol = colBlock * blockSize;
        
        int endRow = std::min(startRow + blockSize, A.getRows());
        int endCol = std::min(startCol + blockSize, C.getCols());
        int common = B.getRows();
        
        for (int i = startRow; i < endRow; ++i) {
            for (int j = startCol; j < endCol; ++j) {
                int sum = 0;
                for (int k = 0; k < common; ++k) {
                    sum += A.get(i, k) * B.get(k, j);
                }
                
                std::lock_guard<std::mutex> lock(mtx);
                C.set(i, j, C.get(i, j) + sum);
            }
        }
    }

    void multiplyMatricesParallel() {
        int numRowBlocks = (A.getRows() + blockSize - 1) / blockSize;
        int numColBlocks = (C.getCols() + blockSize - 1) / blockSize;
        
        std::vector<std::thread> threads;
        
        for (int i = 0; i < numRowBlocks; ++i) {
            for (int j = 0; j < numColBlocks; ++j) {
                threads.emplace_back(&BlockMultiplier::multiplyBlock, this, i, j);
            }
        }
        
        for (auto& t : threads) {
            t.join();
        }
    }
};

Matrix multiplyMatricesSimple(const Matrix& A, const Matrix& B) {
    int n = A.getRows();
    int m = A.getCols();
    int p = B.getCols();
    
    Matrix C(n, p);
    
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < p; ++j) {
            int sum = 0;
            for (int k = 0; k < m; ++k) {
                sum += A.get(i, k) * B.get(k, j);
            }
            C.set(i, j, sum);
        }
    }
    
    return C;
}

int main() {
    const int N = 200;
    
    std::cout << "Matrix size: " << N << "x" << N << std::endl;
    
    Matrix A(N, N);
    Matrix B(N, N);
    
    A.randomFill();
    B.randomFill();
    
    std::cout << "classical algorithm:\n";
    auto start = std::chrono::high_resolution_clock::now();
    Matrix C_simple = multiplyMatricesSimple(A, B);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration_simple = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "time: " << duration_simple.count() << " ms\n\n";
    
    std::cout << "Multi-threaded algorithm with different block sizes:\n";
    std::cout << std::setw(10) << "Block size" 
              << std::setw(15) << "Num blocks" 
              << std::setw(15) << "Num threads" 
              << std::setw(15) << "Time (ms)" 
              << std::setw(15) << "Speedup" << std::endl;
    std::cout << std::string(70, '-') << std::endl;
    
    for (int k = 1; k <= N; k += 1) {
        Matrix C_parallel(N, N);
        BlockMultiplier multiplier(A, B, C_parallel, k);
        
        int numRowBlocks = (N + k - 1) / k;
        int numColBlocks = (N + k - 1) / k;
        int numBlocks = numRowBlocks * numColBlocks;
        int numThreads = numBlocks; 
        
        start = std::chrono::high_resolution_clock::now();
        multiplier.multiplyMatricesParallel();
        end = std::chrono::high_resolution_clock::now();
        auto duration_parallel = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        double speedup = static_cast<double>(duration_simple.count()) / duration_parallel.count();
        
        std::cout << std::setw(10) << k 
                  << std::setw(15) << numBlocks 
                  << std::setw(15) << numThreads 
                  << std::setw(15) << duration_parallel.count()
                  << std::setw(15) << std::fixed << std::setprecision(2) << speedup << std::endl;
    }
    
    return 0;
}