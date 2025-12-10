# Реализация параллельного алгоритма Дейкстры 


## Краткое описание
...

---

## 2. Инструкция по сборке и запуску
### Требования
- Компиляторы:
    - `g++` 
    - `mpic++` (для MPI версии)
- MPI-библиотека для параллельного запуска (`mpirun`)
- Make (`make`) для автоматизации сборки
- Unix-подобная система / WSL 

### Команды
```bash
# Запуск MPI программы
cd ./src
mpic++ ./dijkstra_mpi/dijkstra_mpi.cpp -Wall -o ./dijkstra_mpi/dijkstra_mpi.out
mpiexec ./dijkstra_mpi/dijkstra_mpi.out -n 8

# Запуск обычной программы
cd ./src
g++ ./dijkstra_serial/dijkstra_serial.cpp -Wall -o ./dijkstra_serial/dijkstra_serial.out
./dijkstra_serial/dijkstra_serial.out
```


---

## 3. Измерения производительности для тестовых случаев (размер графа N) по сравнению с последовательной реализацией Dijkstra  
...


## 4. Алгоритмическая часть 
...
