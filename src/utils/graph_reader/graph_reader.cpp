#include "graph_reader.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

/**
 * @brief Считывает квадратную матрицу графа из файла.
 *
 * Формат: n строк, каждая содержит n чисел — веса рёбер.
 * Значение INF означает отсутствие ребра.
 *
 * @param filename Имя файла с матрицей смежности.
 * @param n [out] Размерность матрицы (число вершин).
 * @return Плоский вектор размером n*n, представляющий матрицу.
 */
std::vector<int> readGraphFromFile(const std::string &filename, int &n) {
    std::ifstream ifs(filename);
    if (!ifs) {
        throw std::runtime_error("Ошибка открытия файла " + filename);
    }

    std::vector<std::vector<int>> matrix;
    std::string line;

    while (std::getline(ifs, line)) {
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::vector<int> row;
        int value;

        while (iss >> value) {
            row.push_back(value);
        }

        if (!row.empty()) {
            matrix.push_back(std::move(row));
        }
    }

    if (matrix.empty()) {
        throw std::runtime_error("Файл пуст или некорректен");
    }

    n = (int)matrix.size();

    for (const auto &row : matrix) {
        if ((int)row.size() != n) {
            throw std::runtime_error("Матрица должна быть квадратной");
        }
    }

    std::vector<int> flat(n * n);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            flat[i * n + j] = matrix[i][j];

    return flat;
}
