#pragma once
#include <vector>
#include <string>

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
std::vector<int> readGraphFromFile(const std::string &filename, int &n);
