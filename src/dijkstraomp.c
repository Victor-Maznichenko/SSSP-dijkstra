#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <assert.h>

// Чтение матрицы смежности из файла (файл должен быть в формате txt)
int **readGraphFromFile(const char *filename, int *n) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Ошибка открытия файла %s\n", filename);
        exit(1);
    }

    // Считаем количество строк (вершин)
    int count = 0;
    int ch;
    while ((ch = fgetc(file)) != EOF) {
        if (ch == '\n') count++;
    }
    *n = count;
    rewind(file);

    // Выделяем память под граф
    int **graph = malloc(*n * sizeof(int *));
    for (int i = 0; i < *n; i++) {
        graph[i] = malloc(*n * sizeof(int));
    }

    // Считываем значения
    for (int i = 0; i < *n; i++) {
        for (int j = 0; j < *n; j++) {
            if (fscanf(file, "%d", &graph[i][j]) != 1) {
                printf("Ошибка чтения значения в файле %s\n", filename);
                exit(1);
            }
        }
    }

    fclose(file);
    return graph;
}

// Функция поиска вершины с минимальным расстоянием среди ещё не обработанных
int minDistance(int distances[], int inShortestPathTree[], int countVertices) {
    int min = INT_MAX;
    int min_index = -1;
    for (int v = 0; v < countVertices; v++) {
        if (inShortestPathTree[v] == 0 && distances[v] <= min) {
            min = distances[v];
            min_index = v;
        }
    }
    return min_index;
}

// Алгоритм Дейкстры для нахождения кратчайших путей от исходной вершины ко всем остальным
int *dijkstra(int **graph, int src, int countVertices) {
    int dist[countVertices];               // Массив расстояний от src до i
    int inShortestPathTree[countVertices]; // Флаги включения вершины в дерево кратчайших путей

    // Инициализация расстояний как бесконечность и флагов как false
    for (int i = 0; i < countVertices; i++) {
        dist[i] = INT_MAX;
        inShortestPathTree[i] = 0;
    }
    dist[src] = 0; // Расстояние от исходной вершины до себя равно 0

    // Нахождение кратчайших путей для всех вершин
    for (int count = 0; count < countVertices - 1; count++) {
        int u = minDistance(dist, inShortestPathTree, countVertices);
        inShortestPathTree[u] = 1; // Помечаем вершину как обработанную

        // Обновляем расстояния до смежных вершин
        for (int v = 0; v < countVertices; v++) {
            if (!inShortestPathTree[v] && graph[u][v] && dist[u] != INT_MAX && dist[u] + graph[u][v] < dist[v]) {
                dist[v] = dist[u] + graph[u][v];
            }
        }
    }

    // Выделяем память для возвращаемого массива
    int *distances = (int *)malloc(countVertices * sizeof(int));
    if (distances == NULL) {
        printf("Ошибка выделения памяти.\n");
        exit(1);
    }
    for (int i = 0; i < countVertices; i++) {
        distances[i] = dist[i];
    }
    return distances;
}

// Функция освобождения памяти матрицы
void freeMatrix(int **matrix, int numRows) {
    for (int i = 0; i < numRows; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

int main(int argc, char *argv[]) {
    // Проверка кол-ва входных аргументов
    if (argc != 3) {
        printf("Использование: %s <количество_потоков> [<файл>]\n", argv[0]);
        return 1;
    }

    // Проверка кол-ва потоков thread_count
    char *endptr;
    int threadСount = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0' || threadСount <= 0) {
        printf("Неверное количество потоков.\n");
        return 1;
    }

    int **graph;
	int countVertices;
    printf("Чтение графа из файла: %s\n", argv[2]);
    graph = readGraphFromFile(argv[2], &countVertices);

    // Выделение памяти для матрицы кратчайших путей
    int **shortestPaths = (int **)malloc(countVertices * sizeof(int *));
    if (shortestPaths == NULL) {
        printf("Ошибка выделения памяти.\n");
        exit(1);
    }

    // Начало времени выполнения
    double start = omp_get_wtime();

    // Параллельное вычисление кратчайших путей
    #pragma omp parallel for num_threads(threadСount)
    for (int i = 0; i < countVertices; i++) {
        int *distancesFromSrc = dijkstra(graph, i, countVertices);
        shortestPaths[i] = distancesFromSrc;
    }

    // Конец времени выполнения
    double end = omp_get_wtime();

    // Вывод времени выполнения
    printf("Время вычислений: %.3f ms\n\n", (end - start) * 1000.0);

    // Вывод матрицы смежности графа
    printf("Матрица смежности графа\n");
    for (int i = 0; i < countVertices; i++) {
        for (int j = 0; j < countVertices; j++) {
            printf("%d \t", graph[i][j]);
        }
        printf("\n");
    }

    printf("\n");
    printf("\n");

    // Вывод матрицы кратчайших путей
    printf("Матрица кратчайших путей\n");
    for (int i = 0; i < countVertices; i++) {
        for (int j = 0; j < countVertices; j++) {
            printf("%d \t", shortestPaths[i][j]);
        }
        printf("\n");
    }

    // Освобождение памяти
    freeMatrix(shortestPaths, countVertices);
    freeMatrix(graph, countVertices);

    return 0;
}