#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <chrono>

#define INF INT_MAX

// Чтение матрицы смежности из файла (как в вашем MPI-коде)
int **readGraphFromFile(const char *filename, int *n) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        std::fprintf(stderr, "Ошибка открытия файла %s\n", filename);
        std::exit(1);
    }

    int count = 0;
    int ch;
    while ((ch = fgetc(file)) != EOF) {
        if (ch == '\n') count++;
    }
    *n = count;
    rewind(file);

    if (*n <= 0) {
        std::fprintf(stderr, "Пустой или неверный файл графа\n");
        fclose(file);
        std::exit(1);
    }

    int **graph = (int**)std::malloc(*n * sizeof(int *));
    if (!graph) {
        std::fprintf(stderr, "Ошибка выделения памяти\n");
        fclose(file);
        std::exit(1);
    }
    for (int i = 0; i < *n; i++) {
        graph[i] = (int*)std::malloc(*n * sizeof(int));
        if (!graph[i]) {
            std::fprintf(stderr, "Ошибка выделения памяти\n");
            std::exit(1);
        }
    }

    for (int i = 0; i < *n; i++) {
        for (int j = 0; j < *n; j++) {
            if (fscanf(file, "%d", &graph[i][j]) != 1) {
                std::fprintf(stderr, "Ошибка чтения значения в файле %s (позиция %d,%d)\n", filename, i, j);
                fclose(file);
                std::exit(1);
            }
        }
    }

    fclose(file);
    return graph;
}

// Последовательный Дейкстра
void dijkstra_serial(int **G, int n, int s, int *dist, int *pred) {
    int *visited = (int*)calloc(n, sizeof(int));
    if (!visited) {
        std::fprintf(stderr, "Ошибка выделения памяти visited\n");
        std::exit(1);
    }

    for (int i = 0; i < n; i++) {
        dist[i] = INF;
        pred[i] = -1;
        visited[i] = 0;
    }

    if (s < 0 || s >= n) {
        std::fprintf(stderr, "Неверный стартовый индекс s=%d\n", s);
        free(visited);
        return;
    }
    dist[s] = 0;

    for (int iter = 0; iter < n; iter++) {
        int u = -1;
        int min_dist = INF;
        for (int v = 0; v < n; v++) {
            if (!visited[v] && dist[v] < min_dist) {
                min_dist = dist[v];
                u = v;
            }
        }
        if (u == -1) break;

        visited[u] = 1;

        for (int v = 0; v < n; v++) {
            int w = G[u][v];
            if (!visited[v] && w != INF && dist[u] != INF) {
                if (dist[u] <= INF - w) {
                    int new_dist = dist[u] + w;
                    if (new_dist < dist[v]) {
                        dist[v] = new_dist;
                        pred[v] = u;
                    }
                }
            }
        }
    }

    free(visited);
}

void print_path(int *pred, int s, int v) {
    if (v == s) {
        printf("%d", s);
    } else if (pred[v] == -1) {
        printf("нет пути");
    } else {
        print_path(pred, s, pred[v]);
        printf(" -> %d", v);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::printf("Использование: %s <путь_к_файлу>\n", argv[0]);
        return 1;
    }

    const char *filepath = argv[1];
    int n = 0;
    int **adj = readGraphFromFile(filepath, &n);

    int *dist = (int*)std::malloc(n * sizeof(int));
    int *pred = (int*)std::malloc(n * sizeof(int));
    if (!dist || !pred) {
        std::fprintf(stderr, "Ошибка выделения памяти dist/pred\n");
        return 1;
    }

    int start = 0; // как в вашем параллельном варианте

    // --- измеряем только время вычислений Дейкстры (включая все вычисления внутри функции)
    auto t0 = std::chrono::steady_clock::now();
    dijkstra_serial(adj, n, start, dist, pred);
    auto t1 = std::chrono::steady_clock::now();

    double compute_ms = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(t1 - t0).count();
    std::printf("Compute time: %.4f ms\n", compute_ms);


    // Вывод матрицы смежности графа
    // for (int v = 0; v < n; v++) {
    //     printf("Расстояние до %d = %d, путь: ", v, dist[v]);
    //     print_path(pred, start, v);
    //     printf("\n");
    // }

    // очистка
    for (int i = 0; i < n; i++) std::free(adj[i]);
    std::free(adj);
    std::free(dist);
    std::free(pred);

    return 0;
}
