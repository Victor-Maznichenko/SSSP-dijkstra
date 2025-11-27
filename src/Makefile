CC     := gcc
CXX    := g++
MPICXX := mpic++
CXXFLAGS_COMMON := -std=c++17 -I./utils
CXXFLAGS_DEBUG   := $(CXXFLAGS_COMMON) -g

BUILD_DIR := build

SRC_SERIAL := dijkstra_serial/dijkstra_serial.cpp utils/graph_reader/graph_reader.cpp
SRC_MPI    := dijkstra_mpi/dijkstra_mpi.cpp utils/graph_reader/graph_reader.cpp
GEN_SRC    := graphs_data/generate_graph.c
GEN_BIN    := $(BUILD_DIR)/generate_graph

OBJ_SERIAL := $(patsubst %.cpp,$(BUILD_DIR)/serial/%.o,$(SRC_SERIAL))
OBJ_MPI    := $(patsubst %.cpp,$(BUILD_DIR)/mpi/%.o,$(SRC_MPI))

TARGET_SERIAL := $(BUILD_DIR)/dijkstra_serial/dijkstra_serial
TARGET_MPI    := $(BUILD_DIR)/dijkstra_mpi/dijkstra_mpi

.PHONY: all clean run_serial run_mpi debug release generate_graph

all: $(TARGET_SERIAL) $(TARGET_MPI)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Serial objects
$(BUILD_DIR)/serial/%.o: %.cpp | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS_COMMON) -c $< -o $@

# MPI objects
$(BUILD_DIR)/mpi/%.o: %.cpp | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(MPICXX) $(CXXFLAGS_MPI) -c $< -o $@

# Linking
$(TARGET_SERIAL): $(OBJ_SERIAL)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS_COMMON) -o $@ $^

$(TARGET_MPI): $(OBJ_MPI)
	@mkdir -p $(dir $@)
	$(MPICXX) $(CXXFLAGS_MPI) -o $@ $^

# Генерация графа
$(GEN_BIN): $(GEN_SRC) | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $< -o $@

generate_graph: $(GEN_BIN)
	$(GEN_BIN) ./graphs_data/graph_$(size).txt $(size)

# Параметры запуска
size ?= 20000
kernels ?= 8

# Запуски
run_serial: $(TARGET_SERIAL)
	./$(TARGET_SERIAL) ./graphs_data/graph_$(size).txt

run_mpi: $(TARGET_MPI)
	mpirun -np $(kernels) ./$(TARGET_MPI) ./graphs_data/graph_$(size).txt

debug:
	$(MAKE) CXXFLAGS="$(CXXFLAGS_DEBUG)" clean all

clear:
	rm -rf $(BUILD_DIR)
