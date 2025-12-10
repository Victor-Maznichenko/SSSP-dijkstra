
#### Выделение памяти в C (malloc / calloc)
``` c
int n = 5;

// malloc: выделяет память, но не инициализирует её
int* arr1 = (int*)malloc(n * sizeof(int));

// calloc: выделяет и сразу заполняет нулями
int* arr2 = (int*)calloc(n, sizeof(int));
```

Также в `malloc` / `calloc` нужно указать количество памяти, для этого используем `sizeof(type)`:
| Тип      | `sizeof` (обычно на 64-битной системе) |
| -------- | -------------------------------------- |
| `char`   | 1 байт (8 бит)                         |
| `int`    | 4 байта (32 бита)                      |
| `double` | 8 байт (64 бита)                       |
| `float`  | 4 байта (32 бита)                      |
| `long`   | 8 байт (64 бита, зависит от системы)   |


### Время вычислений
`clock()` (из `<ctime>` / `time.h`)
- Для однопоточной версии работает также точно как и процессорное время из библиотеки `chrono`
- Для вычислений лучше использовать процессорное время (показывает, сколько времени CPU потратил на ваши вычисления, не учитывая паузы, ожидания ввода-вывода или работу других процессов.)



## Компиляторы
mpicc и mpic++ — это обёртки над обычным компилятором (gcc/g++), с подключенным mpi. 
Компиляторы включают в себя версии разных языков.

### C
GCC: `gcc` 
Clang: `clang` 
Intel: `icc` 
MPI: `mpicc` 

### C++
GCC: `g++` 
Clang: `clang++` 
Intel: `icpc`
MPI: `mpic++` 

### Флаги
-Wall включает предупреждения
-std=c++17 указать стандарт языка (иначе по умолчанию встроенный в компилятор будет)
-fopenmp Включение OpenMP
-O2 / -O3 Оптимизация


## Запуск программ
| Тип программы | Команда запуска                                               |
| ------------- | ------------------------------------------------------------- |
| Serial        | `./program`                                                   |
| OpenMP        | `export OMP_NUM_THREADS=4`<br>`./program`                     |
| MPI           | `mpirun -np 4 ./mpi_program`<br>`mpiexec -n 4 ./mpi_program`  |
| MPI + OpenMP  | `export OMP_NUM_THREADS=2`<br>`mpirun -np 2 ./hybrid_program` |


## Запуск (уточнения)
```bash
# Запуск MPI программы
cd ./src
mpic++ ./dijkstra_mpi/dijkstra_mpi.cpp -Wall -o ./dijkstra_mpi/dijkstra_mpi.out
mpiexec ./dijkstra_mpi/dijkstra_mpi.out -n 8
```

```bash
# Запуск обычной программы
cd ./src
g++ ./dijkstra_serial/dijkstra_serial.cpp -Wall -o ./dijkstra_serial/dijkstra_serial.out
./dijkstra_serial//dijkstra_serial.out
```

```bash
# Запуск обычной программы через MPI
cd ./src
mpic++ ./dijkstra_serial/dijkstra_serial.cpp -Wall -o ./dijkstra_serial/dijkstra_serial.out
mpiexec ./dijkstra_serial/dijkstra_serial.out

# Вызовет следующую ошибку:
--------------------------------------------------------------------------
Primary job  terminated normally, but 1 process returned
a non-zero exit code. Per user-direction, the job has been aborted.
--------------------------------------------------------------------------
--------------------------------------------------------------------------
mpiexec noticed that process rank 1 with PID 0 on node Victor exited on signal 9 (Killed).
--------------------------------------------------------------------------
```

## Проверка на правильность работы алгоритма:
MPI:
```bash
mpiexec -np 1 ./dijkstra_mpi/dijkstra_mpi.out 
Compute time: 0.000013282 seconds
Matrix load time: 0.000009541 s
Матрица смежности графа:
0 83 86 77 
83 0 15 93 
86 15 0 35 
77 93 35 0 


Расстояния от вершины 0:
0: 0
1: 83
2: 86
3: 77
```

Serial:
```bash
./dijkstra_serial/dijkstra_serial.out
Compute time: 0.000003 seconds
Matrix load time: 0.000005 seconds
Матрица смежности графа:
0 83 86 77
83 0 15 93
86 15 0 35
77 93 35 0


Расстояния от вершины 0:
0: 0
1: 83
2: 86
3: 77
```