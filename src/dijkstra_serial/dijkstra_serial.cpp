#include <iostream>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <limits>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
constexpr int INF = std::numeric_limits<int>::max();

/**
 * @brief Генерация графа со случайными весами, возвращает плоский вектор n*n (row-major).
 * @param countVertices Количество вершин.
 */
std::vector<int> generateGraph(int countVertices) {
    std::srand(0); 

    int n = countVertices;
    std::vector<int> flat(n * n);

    for (int i = 0; i < n; ++i) {
        for (int j = i; j < n; ++j) {
            int value;
            if (i == j) {
                value = 0;
            } else {
                value = std::rand() % 100;
            }

            // симметрия для неориентированного графа
            flat[i * n + j] = value;
            flat[j * n + i] = value; 
        }
    }

    return flat;
}


/**
 * @brief Последовательный алгоритм Дейкстры.
 *
 * @param graph Плоская матрица смежности графа.
 * @param n Количество вершин.
 * @param start Стартовая вершина.
 * @param dist [out] Массив кратчайших расстояний.
 * @param pred [out] Массив предшественников для восстановления пути.
 */
void dijkstra_serial(const std::vector<int> &graph, int n, int start, int *dist, int *pred) {
    auto weight = [&](int u, int v) { return graph[u * n + v]; };

    std::vector<int> visited(n, 0);

    for (int i = 0; i < n; i++) {
        dist[i] = INF;
        pred[i] = -1;
    }

    dist[start] = 0;

    for (int iteration = 0; iteration < n; iteration++) {
        int chosen = -1;
        int best_dist = INF;

        for (int v = 0; v < n; v++) {
            if (!visited[v] && dist[v] < best_dist) {
                best_dist = dist[v];
                chosen = v;
            }
        }

        if (chosen == -1) break;
        visited[chosen] = 1;

        for (int v = 0; v < n; v++) {
            int w = weight(chosen, v);
            if (!visited[v] && w != INF) {
                if (dist[chosen] <= INF - w) {
                    int new_dist = dist[chosen] + w;
                    if (new_dist < dist[v]) {
                        dist[v] = new_dist;
                        pred[v] = chosen;
                    }
                }
            }
        }
    }
}

int main(int argc, char *argv[]) {
    int total_nodes = 200;

    // Проверяем, передан ли параметр
    if (argc > 1) {
        try {
            total_nodes = std::stoi(argv[1]);
        } catch (const std::exception &e) {
            std::cerr << "Некорректный параметр total_nodes: " << argv[1] << "\n";
            return 1;
        }
    }

    clock_t read_start = clock();
    std::vector<int> graph_matrix = generateGraph(total_nodes);

    int *dist = (int*)std::malloc(total_nodes * sizeof(int));
    int *pred = (int*)std::malloc(total_nodes * sizeof(int));

    if (!dist || !pred) {
        std::fprintf(stderr, "Ошибка выделения памяти dist/pred\n");
        return 1;
    }

    int start_vertex = 0;
    clock_t compute_start = clock();
    dijkstra_serial(graph_matrix, total_nodes, start_vertex, dist, pred);
    clock_t compute_end = clock();
    
    // ========================================================================================
    // Вывод
    // ========================================================================================
    double compute_time_sec = double(compute_end - compute_start) / CLOCKS_PER_SEC;
    double read_time_sec = double(compute_start - read_start) / CLOCKS_PER_SEC;
    std::printf("total_nodes: %d \n", total_nodes);
    std::printf("Compute time: %.6f seconds\n", compute_time_sec);
    std::printf("Matrix load time: %.6f s\n\n", read_time_sec);

    // Для вывода матрицы смежности графа — раскомментировать:
    // std::cout << "Graph adjacency matrix:\n";
    // for (int row = 0; row < total_nodes; ++row) {
    //     for (int col = 0; col < total_nodes; ++col) {
    //         if (graph_matrix[row * total_nodes + col] == INF)
    //             std::cout << "INF ";
    //         else
    //             std::cout << graph_matrix[row * total_nodes + col] << " ";
    //     }
    //     std::cout << "\n";
    // }
    // std::cout << "\n";

    // Для вывода путей — раскомментировать:
    // std::cout << "The distance from the vertex is 0:\n";
    // for (int v = 0; v < total_nodes; ++v) {
    //     if (dist[v] == INF)
    //         std::cout << v << ": INF\n";
    //     else
    //         std::cout << v << ": " << dist[v] << "\n";
    // }

    std::free(dist);
    std::free(pred);

    return 0;
}
