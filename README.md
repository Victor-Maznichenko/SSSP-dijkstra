# Реализация параллельного алгоритма Дейкстры 


## Краткое описание
...

---

## 2. Инструкция по сборке и запуску
### Требования
- Компиляторы:
    - `g++` 
    - `mpic++`
- MPI-библиотека для параллельного запуска (`mpirun`)
- Make (`make`) для автоматизации сборки
- Unix-подобная система / WSL 

### Команды
#### Запуск с помощью makefile
```bash
# Перейти в папку src
cd ./src

# Запуск MPI программы
make run_mpi NP=<кол-во ядер>

# Запуск обычной программы
make run_serial 

# Очистка скомпилированных программ
make clean 
```


#### Обычный запуск (суперкомпьютер)
```bash
# Перейти в папку src
cd ./src

# Запуск MPI программы
mpic++ ./dijkstra_mpi/dijkstra_mpi.cpp -Wall -o ./dijkstra_mpi/dijkstra_mpi.out
mpiexec ./dijkstra_mpi/dijkstra_mpi.out -n <кол-во ядер>

# Запуск обычной программы
g++ ./dijkstra_serial/dijkstra_serial.cpp -Wall -o ./dijkstra_serial/dijkstra_serial.out
./dijkstra_serial/dijkstra_serial.out
```


---

## 3. Измерения производительности для тестовых случаев (размер графа N) по сравнению с последовательной реализацией Dijkstra  
| Total Nodes | dijkstra_serial (s) | dijkstra_mpi -np 1 (s) | dijkstra_mpi -np 4 (s) | dijkstra_mpi -np 8 (s) |
| ----------: | ------------------: | ---------------------: | ---------------------: | ---------------------: |
|         200 |            0.000594 |               0.000570 |               0.001003 |               0.002181 |
|        2000 |            0.034045 |               0.029458 |               0.039661 |               0.409503 |
|       20000 |            3.881837 |               3.030587 |               2.515687 |               2.477684 |

## 4. Проверка на правильность работы алгоритма:
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

## От запуска к запуску результаты сильно могут отличаться (возможно из-за кэша процессора?)
```bash
victo@Victor:/mnt/c/Users/victo/OneDrive/Рабочий стол/Education/diplom/practice/SSSP-dijkstra/src$ mpiexec -np 4 ./dijkstra_mpi/dijkstra_mpi.out 30000
total_nodes: 30000 
Compute time: 5.526386 seconds
Matrix load time: 21.106766 s
```

```bash
victo@Victor:/mnt/c/Users/victo/OneDrive/Рабочий стол/Education/diplom/practice/SSSP-dijkstra/src$ mpiexec -np 8 ./dijkstra_mpi/dijkstra_mpi.out 30000
total_nodes: 30000 
Compute time: 5.861026 seconds
Matrix load time: 21.960471 s
```

```bash
victo@Victor:/mnt/c/Users/victo/OneDrive/Рабочий стол/Education/diplom/practice/SSSP-dijkstra/src$ mpiexec -np 8 ./dijkstra_mpi/dijkstra_mpi.out 30000
total_nodes: 30000 
Compute time: 4.963197 seconds
Matrix load time: 25.180927 s
```

## Сначала казалось бы получаю плавный прирост, но после идут странные результаты:
```
victo@Victor:/mnt/c/Users/victo/OneDrive/Рабочий стол/Education/diplom/practice/SSSP-dijkstra/src$ mpiexec -np 4 ./dijkstra_mpi/dijkstra_mpi.out 30000
total_nodes: 30000 
Compute time: 5.526386 seconds
Matrix load time: 21.106766 s

victo@Victor:/mnt/c/Users/victo/OneDrive/Рабочий стол/Education/diplom/practice/SSSP-dijkstra/src$ mpiexec -np 8 ./dijkstra_mpi/dijkstra_mpi.out 30000
total_nodes: 30000 
Compute time: 5.861026 seconds
Matrix load time: 21.960471 s

victo@Victor:/mnt/c/Users/victo/OneDrive/Рабочий стол/Education/diplom/practice/SSSP-dijkstra/src$ mpiexec -np 8 ./dijkstra_mpi/dijkstra_mpi.out 30000
total_nodes: 30000 
Compute time: 4.963197 seconds
Matrix load time: 25.180927 s

victo@Victor:/mnt/c/Users/victo/OneDrive/Рабочий стол/Education/diplom/practice/SSSP-dijkstra/src$ mpiexec -np 2 ./dijkstra_mpi/dijkstra_mpi.out 30000
total_nodes: 30000 
Compute time: 5.440561 seconds
Matrix load time: 19.399003 s

victo@Victor:/mnt/c/Users/victo/OneDrive/Рабочий стол/Education/diplom/practice/SSSP-dijkstra/src$ mpiexec -np 1 ./dijkstra_mpi/dijkstra_mpi.out 30000
total_nodes: 30000 
Compute time: 7.095762 seconds
Matrix load time: 22.146261 s

victo@Victor:/mnt/c/Users/victo/OneDrive/Рабочий стол/Education/diplom/practice/SSSP-dijkstra/src$ ./dijkstra_serial/dijkstra_serial.out 30000
total_nodes: 30000 
Compute time: 8.563331 seconds
Matrix load time: 18.032995 s

```

## Необходимо попробовать сравнить результаты с этой программой (если это целесообразно и наши программы делают одно и то же):
[https://github.com/Lehmannhen/MPI-Dijkstra/tree/master](https://github.com/Lehmannhen/MPI-Dijkstra/tree/master)


## 5. Алгоритмическая часть 
...
