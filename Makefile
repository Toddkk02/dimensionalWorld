# --- DIRECTORIES ---
SRC_DIR = src
BUILD_DIR = build
CORE_DIR = $(SRC_DIR)/core
WORLD_DIR = $(SRC_DIR)/world
RENDERING_DIR = $(SRC_DIR)/rendering
GAMEPLAY_DIR = $(SRC_DIR)/gameplay
HORROR_DIR = $(SRC_DIR)/horror

# --- COMPILER & FLAGS ---
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I./include -g -O0
LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -lstdc++

# --- SOURCE FILES ---
MAIN_SRC = $(SRC_DIR)/main.cpp
CORE_SRC = $(wildcard $(CORE_DIR)/*.cpp)
RENDERING_SRC = $(wildcard $(RENDERING_DIR)/*.cpp)
GAMEPLAY_SRC = $(wildcard $(GAMEPLAY_DIR)/*.cpp)
HORROR_SRC = $(wildcard $(HORROR_DIR)/*.cpp)

# Escludi blocks.cpp dalla wildcard del world, lo aggiungiamo manualmente
WORLD_SRC = $(filter-out $(WORLD_DIR)/blocks.cpp, $(wildcard $(WORLD_DIR)/*.cpp))
BLOCKS_SRC = $(WORLD_DIR)/blocks.cpp

SOURCES = $(MAIN_SRC) $(CORE_SRC) $(WORLD_SRC) $(BLOCKS_SRC) $(RENDERING_SRC) $(GAMEPLAY_SRC) $(HORROR_SRC)

# --- OBJECT FILES ---
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SOURCES))

# --- TARGET EXECUTABLE ---
TARGET = $(BUILD_DIR)/game

# --- DEFAULT TARGET ---
all: $(TARGET)

# --- LINKING ---
$(TARGET): $(OBJECTS)
	@echo "=========================================="
	@echo "   LINKING DIMENSIONAL WORLD"
	@echo "=========================================="
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo "✓ Build successful!"
	@echo "Run with: make run"

# --- COMPILE RULE ---
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# --- CLEAN ---
clean:
	@echo "Cleaning build directory..."
	rm -rf $(BUILD_DIR)
	@echo "✓ Clean complete"

# --- RUN ---
run: $(TARGET)
	@echo "=========================================="
	@echo "   STARTING DIMENSIONAL WORLD"
	@echo "=========================================="
	./$(TARGET)

# --- DEBUG BUILD ---
debug: CXXFLAGS += -g -DDEBUG
debug: clean $(TARGET)
	@echo "✓ Debug build complete"

# --- HELP ---
help:
	@echo "=========================================="
	@echo "   DIMENSIONAL WORLD - BUILD SYSTEM"
	@echo "=========================================="
	@echo "Available targets:"
	@echo "  make          - Build the game (default)"
	@echo "  make run      - Build and run the game"
	@echo "  make clean    - Remove build files"
	@echo "  make debug    - Build with debug symbols"
	@echo "  make help     - Show this help message"
	@echo "=========================================="

.PHONY: all clean run debug help
