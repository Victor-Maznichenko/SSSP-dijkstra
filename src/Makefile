# Пути к исходникам и исполняемым файлам
MPI_SRC = ./dijkstra_mpi/dijkstra_mpi.cpp
MPI_OUT = ./dijkstra_mpi/dijkstra_mpi.out

SERIAL_SRC = ./dijkstra_serial/dijkstra_serial.cpp
SERIAL_OUT = ./dijkstra_serial/dijkstra_serial.out

# Компиляторы и флаги
MPICXX = mpic++
CXX = g++
CXXFLAGS = -Wall

# Количество процессов по умолчанию для MPI
NP ?= 4

# По умолчанию: сборка обоих проектов
all: $(MPI_OUT) $(SERIAL_OUT)

# Сборка MPI версии
$(MPI_OUT): $(MPI_SRC)
	$(MPICXX) $(CXXFLAGS) $(MPI_SRC) -o $(MPI_OUT)

# Сборка серийной версии
$(SERIAL_OUT): $(SERIAL_SRC)
	$(CXX) $(CXXFLAGS) $(SERIAL_SRC) -o $(SERIAL_OUT)

# Запуск серийной версии
run_serial: $(SERIAL_OUT)
	./$(SERIAL_OUT)

# Запуск MPI версии с переменной NP
run_mpi: $(MPI_OUT)
	mpiexec -np $(NP) ./$(MPI_OUT)

# Очистка собранных файлов
clean:
	rm -f $(MPI_OUT) $(SERIAL_OUT)
