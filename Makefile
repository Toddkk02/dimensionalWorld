# --- DIRECTORIES ---
SRC_DIR = src
BUILD_DIR = build
CORE_DIR = $(SRC_DIR)/core
WORLD_DIR = $(SRC_DIR)/world
RENDERING_DIR = $(SRC_DIR)/rendering
GAMEPLAY_DIR = $(SRC_DIR)/gameplay

# --- COMPILER & FLAGS ---
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I./include
LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -lstdc++

# --- SOURCE FILES ---
MAIN_SRC = $(SRC_DIR)/main.cpp
CORE_SRC = $(wildcard $(CORE_DIR)/*.cpp)
RENDERING_SRC = $(wildcard $(RENDERING_DIR)/*.cpp)
GAMEPLAY_SRC = $(wildcard $(GAMEPLAY_DIR)/*.cpp)

# Escludi blocks.cpp dalla wildcard del world, lo aggiungiamo manualmente
WORLD_SRC = $(filter-out $(WORLD_DIR)/blocks.cpp, $(wildcard $(WORLD_DIR)/*.cpp))
BLOCKS_SRC = $(WORLD_DIR)/blocks.cpp

SOURCES = $(MAIN_SRC) $(CORE_SRC) $(WORLD_SRC) $(BLOCKS_SRC) $(RENDERING_SRC) $(GAMEPLAY_SRC)

# --- OBJECT FILES ---
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SOURCES))

# --- TARGET EXECUTABLE ---
TARGET = $(BUILD_DIR)/game

# --- DEFAULT TARGET ---
all: $(TARGET)

# --- LINKING ---
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# --- COMPILE RULE ---
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# --- CLEAN ---
clean:
	rm -rf $(BUILD_DIR)

# --- RUN ---
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
