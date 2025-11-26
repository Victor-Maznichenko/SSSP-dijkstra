#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

// Инициализация графа со случайными весами
int **initGraf(int countVertices) {
    srand(13517143);

    int **graph = (int **)malloc(countVertices * sizeof(int *));
    if (graph == NULL) {
        printf("Ошибка выделения памяти.\n");
        exit(1);
    }

    for (int i = 0; i < countVertices; i++) {
        graph[i] = (int *)malloc(countVertices * sizeof(int));
        if (graph[i] == NULL) {
            printf("Ошибка выделения памяти.\n");
            exit(1);
        }
    }

    for (int i = 0; i < countVertices; i++) {
        for (int j = i; j < countVertices; j++) {
            int random = rand() % 100;
            if (i == j) {
                graph[i][i] = 0;
            } else {
                graph[i][j] = random;
                graph[j][i] = random;
            }
        }
    }

    return graph;
}

// Функция для записи матрицы в файл
void saveGraphToFile(int **graph, int countVertices, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        printf("Ошибка открытия файла %s\n", filename);
        exit(1);
    }

    for (int i = 0; i < countVertices; i++) {
        for (int j = 0; j < countVertices; j++) {
            fprintf(file, "%d ", graph[i][j]);
        }
        fprintf(file, "\n");
    }

    fclose(file);
}

// Освобождение памяти
void freeGraph(int **graph, int countVertices) {
    for (int i = 0; i < countVertices; i++) {
        free(graph[i]);
    }
    free(graph);
}

int main() {
    // Создаём папку, если её нет
    const char *dir = "./src/test_graphs";
    mkdir("./src", 0777);
    mkdir(dir, 0777);

    int sizes[] = {100, 1000, 20000, 25000};
    char filenames[4][64];

    for (int i = 0; i < 4; i++) {
        int n = sizes[i];
        printf("Создание графа размером %d...\n", n);

        int **graph = initGraf(n);

        // Формируем путь к файлу в папке src/test_graphs
        snprintf(filenames[i], sizeof(filenames[i]), "%s/graph_%d.txt", dir, n);

        saveGraphToFile(graph, n, filenames[i]);
        printf("Граф сохранён в %s\n", filenames[i]);

        freeGraph(graph, n);
    }

    printf("Все графы успешно созданы и сохранены в %s\n", dir);
    return 0;
}
