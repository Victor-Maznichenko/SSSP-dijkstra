#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <chrono>

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "../utils/graph_reader/graph_reader.hpp"


#define INF INT_MAX

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
    if (argc < 2) {
        std::printf("Использование: %s <путь_к_файлу>\n", argv[0]);
        return 1;
    }

    int n = 0;
    std::string filepath = argv[1];

    auto read_start = std::chrono::steady_clock::now();
    std::vector<int> adjacency = readGraphFromFile(filepath, n);

    int *dist = (int*)std::malloc(n * sizeof(int));
    int *pred = (int*)std::malloc(n * sizeof(int));

    if (!dist || !pred) {
        std::fprintf(stderr, "Ошибка выделения памяти dist/pred\n");
        return 1;
    }

    int start_vertex = 0;
    auto compute_start = std::chrono::steady_clock::now();

    dijkstra_serial(adjacency, n, start_vertex, dist, pred);

    auto compute_end = std::chrono::steady_clock::now();

    double compute_time_ms = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(compute_end - compute_start).count();
    printf("Compute time: %.9f seconds\n", compute_time_ms / 1000.0);

    double read_time_ms =
        std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(compute_start - read_start).count();
    printf("Matrix load time: %.9f s\n", read_time_ms / 1000.0);

    // Для вывода путей — раскомментировать:
    /*
    for (int v = 0; v < n; v++) {
        printf("Расстояние до %d = %d, путь: ", v, dist[v]);
        print_path(pred, start_vertex, v);
        printf("\n");
    }
    */

    std::free(dist);
    std::free(pred);

    return 0;
}
