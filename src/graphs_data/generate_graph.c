#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <sys/stat.h>

// Инициализация графа со случайными весами
std::vector<std::vector<int>> initGraph(int countVertices) {
    std::srand(13517143); // фиксируем seed

    std::vector<std::vector<int>> graph(countVertices, std::vector<int>(countVertices));

    for (int i = 0; i < countVertices; ++i) {
        for (int j = i; j < countVertices; ++j) {
            int random = std::rand() % 100;
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
void saveGraphToFile(const std::vector<std::vector<int>>& graph, const std::string& filename) {
    std::ofstream file(filename);
    if (!file) {
        std::cerr << "Ошибка открытия файла " << filename << std::endl;
        std::exit(1);
    }

    for (const auto& row : graph) {
        for (int val : row) {
            file << val << " ";
        }
        file << "\n";
    }
}

int main() {
    // Создаём папку, если её нет
    const std::string dir = "./graphs_data";
    mkdir(dir.c_str(), 0777);

    int sizes[] = {10, 5000, 20000, 25000};

    for (int n : sizes) {
        std::cout << "Создание графа размером " << n << "...\n";

        auto graph = initGraph(n);

        std::string filename = dir + "/graph_" + std::to_string(n) + ".txt";

        saveGraphToFile(graph, filename);
        std::cout << "Граф сохранён в " << filename << std::endl;
    }

    std::cout << "Все графы успешно созданы и сохранены в " << dir << std::endl;
    return 0;
}
