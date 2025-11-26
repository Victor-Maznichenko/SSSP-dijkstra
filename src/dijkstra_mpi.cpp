#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <limits>
#include <chrono>
#include <algorithm>
#include <array>      // <- для std::array
#include <cstring>    // <- для std::memcpy
#include <stdexcept>  // <- для std::runtime_error
#include <cstdio>     // <- для printf (если оставляете std::printf)
#include <mpi.h>

constexpr int INF = std::numeric_limits<int>::max();

// Чтение матрицы смежности из файла (возвращает плоский вектор размера n*n)
std::vector<int> readGraphFromFile(const std::string &filename, int &n) {
    std::ifstream ifs(filename);
    if (!ifs) {
        throw std::runtime_error("Ошибка открытия файла " + filename);
    }

    std::vector<std::vector<int>> rows;
    std::string line;
    while (std::getline(ifs, line)) {
        if (line.empty()) continue;
        std::istringstream iss(line);
        std::vector<int> row;
        int val;
        while (iss >> val) row.push_back(val);
        if (!row.empty()) rows.push_back(std::move(row));
    }

    if (rows.empty()) {
        throw std::runtime_error("Файл пуст или не содержит корректных строк");
    }

    // Проверим, что матрица квадратная
    n = static_cast<int>(rows.size());
    for (const auto &r : rows) {
        if (static_cast<int>(r.size()) != n) {
            throw std::runtime_error("Матрица должна быть квадратной (n x n)");
        }
    }

    std::vector<int> flat(n * n);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            flat[i * n + j] = rows[i][j];
        }
    }
    return flat;
}

void dijkstra_mpi(int *loc_adj_ptr, int *loc_dist_ptr, int *loc_pred_ptr, int n, int loc_n, int start, MPI_Comm comm) {
    int my_rank = 0, procs = 1;
    MPI_Comm_rank(comm, &my_rank);
    MPI_Comm_size(comm, &procs);

    // Обернём сырой указатель во вектороподобный интерфейс (только для удобства чтения)
    int *loc_adj = loc_adj_ptr;
    int *loc_dist = loc_dist_ptr;
    int *loc_pred = loc_pred_ptr;

    std::vector<int> loc_visited(loc_n, 0);
    std::array<int,2> global_min{INF, -1};
    std::array<int,2> local_min{INF, -1};
    std::vector<int> u_row(n);

    // Инициализация
    for (int i = 0; i < loc_n; ++i) {
        loc_dist[i] = INF;
        loc_pred[i] = -1;
    }

    int my_start_begin = my_rank * loc_n;
    int my_start_end = my_start_begin + loc_n;
    if (start >= my_start_begin && start < my_start_end) {
        int local_start = start - my_start_begin;
        loc_dist[local_start] = 0;
    }

    for (int iter = 0; iter < n; ++iter) {
        local_min = {INF, -1};

        for (int i = 0; i < loc_n; ++i) {
            if (!loc_visited[i] && loc_dist[i] < local_min[0]) {
                local_min[0] = loc_dist[i];
                local_min[1] = my_rank * loc_n + i;
            }
        }

        // MPI_Allreduce с типом MPI_2INT и оператором MPI_MINLOC
        MPI_Allreduce(local_min.data(), global_min.data(), 1, MPI_2INT, MPI_MINLOC, comm);

        int u = global_min[1];
        if (u == -1) break;

        int owner = u / loc_n;
        int local_u = u % loc_n;
        if (my_rank == owner) loc_visited[local_u] = 1;

        int current_dist = global_min[0];
        if (my_rank == owner) {
            // у строки локально — копируем её в u_row
            std::memcpy(u_row.data(), &loc_adj[local_u * n], n * sizeof(int));
        }

        MPI_Bcast(&current_dist, 1, MPI_INT, owner, comm);
        MPI_Bcast(u_row.data(), n, MPI_INT, owner, comm);

        for (int local_v = 0; local_v < loc_n; ++local_v) {
            if (!loc_visited[local_v]) {
                int global_v = my_rank * loc_n + local_v;
                int w = u_row[global_v];
                if (w != INF && current_dist != INF && current_dist <= INF - w) {
                    int new_dist = current_dist + w;
                    if (new_dist < loc_dist[local_v]) {
                        loc_dist[local_v] = new_dist;
                        loc_pred[local_v] = u;
                    }
                }
            }
        }
    }
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int my_rank = 0, procs = 1;
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Comm_rank(comm, &my_rank);
    MPI_Comm_size(comm, &procs);

    if (argc < 2) {
        if (my_rank == 0) {
            std::cout << "Использование: " << argv[0] << " <путь_к_файлу>\n";
        }
        MPI_Finalize();
        return 1;
    }

    std::string filepath = argv[1];
    int n = 0;
    std::vector<int> adj_full;

    if (my_rank == 0) {
        try {
            adj_full = readGraphFromFile(filepath, n);
        } catch (const std::exception &ex) {
            std::cerr << "Ошибка: " << ex.what() << '\n';
            MPI_Abort(comm, 1);
        }
    }

    // Передаём размер графа всем процессам
    MPI_Bcast(&n, 1, MPI_INT, 0, comm);

    if (procs > n || n % procs != 0) {
        if (my_rank == 0) {
            std::cerr << "Ошибка: n должно делиться на число процессов и быть >= их количества\n";
        }
        MPI_Finalize();
        return 1;
    }

    int loc_n = n / procs;
    MPI_Barrier(comm);
    double parallel_start = MPI_Wtime();

    // Локальные буферы
    std::vector<int> loc_adj(loc_n * n);
    std::vector<int> loc_dist(loc_n);
    std::vector<int> loc_pred(loc_n);

    if (my_rank == 0) {
        // Разбиваем плоскую матрицу по процессам (строчно)
        for (int p = 0; p < procs; ++p) {
            int offset = p * loc_n * n; // начало блока строк для процесса p
            if (p == 0) {
                std::memcpy(loc_adj.data(), adj_full.data() + offset, loc_n * n * sizeof(int));
            } else {
                MPI_Send(adj_full.data() + offset, loc_n * n, MPI_INT, p, 0, comm);
            }
        }
        // adj_full выйдет из scope и освободится автоматически
    } else {
        MPI_Recv(loc_adj.data(), loc_n * n, MPI_INT, 0, 0, comm, MPI_STATUS_IGNORE);
    }

    MPI_Barrier(comm);
    auto t0 = std::chrono::steady_clock::now();
    dijkstra_mpi(loc_adj.data(), loc_dist.data(), loc_pred.data(), n, loc_n, 0, comm);
    auto t1 = std::chrono::steady_clock::now();
    double dijkstra_end = MPI_Wtime();

    std::vector<int> global_dist, global_pred;
    if (my_rank == 0) {
        global_dist.resize(n);
        global_pred.resize(n);
    }

    MPI_Gather(loc_dist.data(), loc_n, MPI_INT,
               my_rank == 0 ? global_dist.data() : nullptr, loc_n, MPI_INT,
               0, comm);
    MPI_Gather(loc_pred.data(), loc_n, MPI_INT,
               my_rank == 0 ? global_pred.data() : nullptr, loc_n, MPI_INT,
               0, comm);

    double parallel_end = MPI_Wtime();

    if (my_rank == 0) {
        double compute_ms = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(t1 - t0).count();
        std::printf("Compute time: %.3f ms\n", compute_ms);

        // std::cout << "Vertex\tDist\tPred\n";
        // for (int i = 0; i < n; ++i) {
        //     if (global_dist[i] == INF) std::cout << i << "\tINF\t" << global_pred[i] << "\n";
        //     else std::cout << i << "\t" << global_dist[i] << "\t" << global_pred[i] << "\n";
        // }
    }

    MPI_Finalize();
    return 0;
}
