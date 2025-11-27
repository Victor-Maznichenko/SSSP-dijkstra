#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <algorithm>
#include <array>
#include <cstring>
#include <stdexcept>
#include <cstdio>
#include <mpi.h>

#include "../utils/graph_reader/graph_reader.hpp"

constexpr int INF = std::numeric_limits<int>::max();

/**
 * @brief Параллельная реализация алгоритма Дейкстры с использованием MPI.
 *
 * @param loc_adj_ptr Указатель на локальную подматрицу смежности (строчно разбита по процессам).
 * @param loc_dist_ptr Указатель на массив локальных расстояний (размер = rows_per_proc).
 * @param loc_pred_ptr Указатель на массив предков (размер = rows_per_proc).
 * @param total_nodes Общее число вершин в графе.
 * @param rows_per_proc Число строк (вершин) на один процесс.
 * @param start Начальная вершина (глобальный индекс).
 * @param comm MPI-коммуникатор.
 */
void dijkstra_mpi(int *local_adj, int *local_dist, int *local_pred, int total_nodes, int rows_per_proc, int start, MPI_Comm comm) {
    int rank = 0, num_procs = 1;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &num_procs);

    // Вспомогательные структуры
    std::vector<int> visited(rows_per_proc, 0);
    std::array<int, 2> global_min_pair{INF, -1}; // {min_dist, global_index}
    std::array<int, 2> local_min_pair{INF, -1};
    std::vector<int> u_row_buffer(total_nodes);

    // Инициализация локальных массивов расстояний и предков
    for (int i = 0; i < rows_per_proc; ++i) {
        local_dist[i] = INF;
        local_pred[i] = -1;
    }

    const int my_block_begin = rank * rows_per_proc;
    const int my_block_end = my_block_begin + rows_per_proc;
    if (start >= my_block_begin && start < my_block_end) {
        int local_start_index = start - my_block_begin;
        local_dist[local_start_index] = 0;
    }

    // Основной цикл: n итераций выбора минимальной вершины
    for (int iter = 0; iter < total_nodes; ++iter) {
        local_min_pair = {INF, -1};

        // Находим минимальную непосещённую локальную вершину
        for (int i = 0; i < rows_per_proc; ++i) {
            if (!visited[i] && local_dist[i] < local_min_pair[0]) {
                local_min_pair[0] = local_dist[i];
                local_min_pair[1] = rank * rows_per_proc + i; // глобальный индекс
            }
        }

        // Сверяем локальные минимумы по всем процессам — используем MPI_MINLOC
        MPI_Allreduce(local_min_pair.data(), global_min_pair.data(), 1, MPI_2INT, MPI_MINLOC, comm);

        int u_global_idx = global_min_pair[1];
        if (u_global_idx == -1) break; // больше недостижимых вершин

        int owner_rank = u_global_idx / rows_per_proc;
        int local_u_idx = u_global_idx % rows_per_proc;
        if (rank == owner_rank) visited[local_u_idx] = 1;

        int current_dist = global_min_pair[0];
        if (rank == owner_rank) {
            // Владелец вершины отправляет всю свою строку смежности
            std::memcpy(u_row_buffer.data(), &local_adj[local_u_idx * total_nodes], total_nodes * sizeof(int));
        }

        // Широковещательно передаём расстояние и строку смежности владельцем
        MPI_Bcast(&current_dist, 1, MPI_INT, owner_rank, comm);
        MPI_Bcast(u_row_buffer.data(), total_nodes, MPI_INT, owner_rank, comm);

        // Обновляем локальные расстояния, используя полученную строку смежности
        for (int local_v = 0; local_v < rows_per_proc; ++local_v) {
            if (!visited[local_v]) {
                int global_v = rank * rows_per_proc + local_v;
                int weight = u_row_buffer[global_v];

                if (weight != INF && current_dist != INF && current_dist <= INF - weight) {
                    int candidate = current_dist + weight;

                    if (candidate < local_dist[local_v]) {
                        local_dist[local_v] = candidate;
                        local_pred[local_v] = u_global_idx;
                    }
                }
            }
        }
    }
}

/**
 * @brief Точка входа: чтение графа, распределение по процессам и запуск Dijkstra.
 *
 * Ожидает один аргумент: путь к файлу с графом. Матрица смежности должна быть
 * представлена как плоский массив целых (row-major).
 */
int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    MPI_Comm comm = MPI_COMM_WORLD;
    int rank = 0, num_procs = 1;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &num_procs);
    MPI_Barrier(comm);

    double read_start_time = MPI_Wtime();

    if (argc < 2) {
        if (rank == 0) {
            std::cout << "Использование: " << argv[0] << " <путь_к_файлу>\n";
        }
        MPI_Finalize();
        return 1;
    }

    std::string filepath = argv[1];
    int total_nodes = 0;
    std::vector<int> adjacency_flat;

    if (rank == 0) {
        try {
            adjacency_flat = readGraphFromFile(filepath, total_nodes);
        } catch (const std::exception &ex) {
            std::cerr << "Ошибка при чтении графа: " << ex.what() << '\n';
            MPI_Abort(comm, 1);
        }
    }

    // Передаём число вершин всем процессам
    MPI_Bcast(&total_nodes, 1, MPI_INT, 0, comm);

    if (num_procs > total_nodes || total_nodes % num_procs != 0) {
        if (rank == 0) {
            std::cerr << "Ошибка: total_nodes должно делиться на число процессов и быть >= их количества\n";
        }
        MPI_Finalize();
        return 1;
    }

    int rows_per_proc = total_nodes / num_procs;
    MPI_Barrier(comm);

    // Локальные буферы для каждого процесса
    std::vector<int> local_adj(rows_per_proc * total_nodes);
    std::vector<int> local_dist(rows_per_proc);
    std::vector<int> local_pred(rows_per_proc);

    // Разбиваем плоскую матрицу по процессам (блоки строк)
    if (rank == 0) {
        for (int p = 0; p < num_procs; ++p) {
            int offset = p * rows_per_proc * total_nodes;

            if (p == 0) {
                std::memcpy(local_adj.data(), adjacency_flat.data() + offset, rows_per_proc * total_nodes * sizeof(int));
                continue;
            }
            
            MPI_Send(adjacency_flat.data() + offset, rows_per_proc * total_nodes, MPI_INT, p, 0, comm);
        }
    } else {
        MPI_Recv(local_adj.data(), rows_per_proc * total_nodes, MPI_INT, 0, 0, comm, MPI_STATUS_IGNORE);
    }

    MPI_Barrier(comm);
    double parallel_start_time = MPI_Wtime();

    // Запуск параллельного Dijkstra по локальному блоку строк
    dijkstra_mpi(local_adj.data(), local_dist.data(), local_pred.data(), total_nodes, rows_per_proc, 0, comm);

    MPI_Barrier(comm);
    double parallel_end_time = MPI_Wtime();

    // Сбор результатов на корневом процессе
    std::vector<int> global_dist, global_pred;
    if (rank == 0) {
        global_dist.resize(total_nodes);
        global_pred.resize(total_nodes);
    }

    int* recv_dist_ptr = (rank == 0) ? global_dist.data() : nullptr;
    int* recv_pred_ptr = (rank == 0) ? global_pred.data() : nullptr;
    MPI_Gather(local_dist.data(), rows_per_proc, MPI_INT, recv_dist_ptr, rows_per_proc, MPI_INT, 0, comm);
    MPI_Gather(local_pred.data(), rows_per_proc, MPI_INT, recv_pred_ptr, rows_per_proc, MPI_INT, 0, comm);

    // Вывод
    if (rank == 0) {
        std::printf("Calc time: %.9f s\n", parallel_end_time - parallel_start_time);
        std::printf("Matrix load time: %.9f s\n", parallel_start_time - read_start_time);
    }

    // Для вывода путей — раскомментировать:
    // if (rank == 0) {
    //     std::cout << "Расстояния от вершины 0:\n";
    //     for (int v = 0; v < total_nodes; ++v) {
    //         if (global_dist[v] == INF)
    //             std::cout << "v" << v << ": INF\n";
    //         else
    //             std::cout << "v" << v << ": " << global_dist[v] << "\n";
    //     }
    // }

    MPI_Finalize();
    return 0;
}
