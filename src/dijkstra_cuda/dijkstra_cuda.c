#include <cuda.h>
#include <cuda_runtime.h>
#include <thrust/reduce.h>
#include <thrust/device_ptr.h>
#include <iostream>
#include <climits>

#define INF INT_MAX

// Ядро алгоритма Дейкстры (Algorithm 4)
__global__ void dijkstra_kernel(int n, int* Adj, int* dist, int* pred, bool* updated) {
    int tid = blockIdx.x * blockDim.x + threadIdx.x;
    
    if (tid >= n) return;
    
    for (int v = 0; v < n; v++) {
        int weight = Adj[tid * n + v];
        
        if (weight != INF && dist[tid] != INF) {
            int new_dist = dist[tid] + weight;
            
            if (new_dist < dist[v]) {
                // Атомарное обновление минимального расстояния
                int old_dist = atomicMin(&dist[v], new_dist);
                
                if (new_dist < old_dist) {
                    pred[v] = tid;
                    updated[v] = true;
                }
            }
        }
    }
}

// Главная функция алгоритма Дейкстры на CUDA (Algorithm 3)
void dijkstraCUDA(int n, int* Adj, int* dist, int* pred, int start_node) {
    // Инициализация массивов на хосте
    for (int i = 0; i < n; i++) {
        dist[i] = INF;
        pred[i] = -1;
    }
    dist[start_node] = 0;
    
    // Указатели для памяти GPU
    int *d_Adj, *d_dist, *d_pred;
    bool *d_updated;
    
    // Выделение памяти на GPU
    cudaMalloc(&d_Adj, n * n * sizeof(int));
    cudaMalloc(&d_dist, n * sizeof(int));
    cudaMalloc(&d_pred, n * sizeof(int));
    cudaMalloc(&d_updated, n * sizeof(bool));
    
    // Копирование данных на GPU
    cudaMemcpy(d_Adj, Adj, n * n * sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(d_dist, dist, n * sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(d_pred, pred, n * sizeof(int), cudaMemcpyHostToDevice);
    
    // Настройка размеров блоков и сетки
    int threadsPerBlock = 256;
    int blocksPerGrid = (n + threadsPerBlock - 1) / threadsPerBlock;
    
    bool anyUpdated;
    
    // Основной цикл алгоритма
    do {
        // Сброс массива обновлений
        cudaMemset(d_updated, false, n * sizeof(bool));
        
        // Запуск ядра
        dijkstra_kernel<<<blocksPerGrid, threadsPerBlock>>>(n, d_Adj, d_dist, d_pred, d_updated);
        cudaDeviceSynchronize();
        
        // Проверка обновлений с помощью Thrust
        thrust::device_ptr<bool> d_updated_ptr(d_updated);
        anyUpdated = thrust::reduce(d_updated_ptr, d_updated_ptr + n, false, thrust::logical_or<bool>());
        
    } while (anyUpdated);
    
    // Копирование результатов обратно на хост
    cudaMemcpy(dist, d_dist, n * sizeof(int), cudaMemcpyDeviceToHost);
    cudaMemcpy(pred, d_pred, n * sizeof(int), cudaMemcpyDeviceToHost);
    
    // Освобождение памяти GPU
    cudaFree(d_Adj);
    cudaFree(d_dist);
    cudaFree(d_pred);
    cudaFree(d_updated);
}

// Вспомогательная функция для создания тестового графа
void createTestGraph(int n, int* Adj) {
    // Инициализация матрицы смежности
    for (int i = 0; i < n * n; i++) {
        Adj[i] = INF;
    }
    
    // Создание простого тестового графа
    // Пример: 4 вершины с некоторыми рёбрами
    if (n >= 4) {
        Adj[0*n + 1] = 2;  // 0 -> 1 вес 2
        Adj[0*n + 2] = 4;  // 0 -> 2 вес 4
        Adj[1*n + 2] = 1;  // 1 -> 2 вес 1
        Adj[1*n + 3] = 7;  // 1 -> 3 вес 7
        Adj[2*n + 3] = 3;  // 2 -> 3 вес 3
    }
}

// Функция для вывода результатов
void printResults(int n, int* dist, int* pred, int start_node) {
    std::cout << "Кратчайшие пути из вершины " << start_node << ":\n";
    for (int i = 0; i < n; i++) {
        std::cout << "Вершина " << i << ": расстояние = " 
                  << (dist[i] == INF ? "INF" : std::to_string(dist[i]))
                  << ", предшественник = " << pred[i] << std::endl;
    }
}

int main() {
    const int n = 4;  // Количество вершин
    int start_node = 0;  // Стартовая вершина
    
    // Выделение памяти для графа и результатов
    int* Adj = new int[n * n];
    int* dist = new int[n];
    int* pred = new int[n];
    
    // Создание тестового графа
    createTestGraph(n, Adj);
    
    // Запуск алгоритма Дейкстры на CUDA
    dijkstraCUDA(n, Adj, dist, pred, start_node);
    
    // Вывод результатов
    printResults(n, dist, pred, start_node);
    
    // Освобождение памяти
    delete[] Adj;
    delete[] dist;
    delete[] pred;
    
    return 0;
}
