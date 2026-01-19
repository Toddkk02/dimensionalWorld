# Compiler
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I./include
LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -lstdc++

# Directories
SRC_DIR = src
BUILD_DIR = build
CORE_DIR = $(SRC_DIR)/core
WORLD_DIR = $(SRC_DIR)/world
RENDERING_DIR = $(SRC_DIR)/rendering
GAMEPLAY_DIR = $(SRC_DIR)/gameplay

# Source files
SOURCES = $(SRC_DIR)/main.cpp \
          $(CORE_DIR)/player.cpp \
          $(CORE_DIR)/portal.cpp \
          $(WORLD_DIR)/decorations.cpp \
          $(WORLD_DIR)/dimensions.cpp \
          $(WORLD_DIR)/firstWorld.cpp \
          $(RENDERING_DIR)/shaders.cpp \
          $(RENDERING_DIR)/skybox.cpp \
          $(GAMEPLAY_DIR)/item.cpp \
          $(GAMEPLAY_DIR)/inventory.cpp \
          $(GAMEPLAY_DIR)/mining.cpp \
          $(GAMEPLAY_DIR)/dropped_item.cpp

# Object files
OBJECTS = $(BUILD_DIR)/main.o \
          $(BUILD_DIR)/core/player.o \
          $(BUILD_DIR)/core/portal.o \
          $(BUILD_DIR)/world/decorations.o \
          $(BUILD_DIR)/world/dimensions.o \
          $(BUILD_DIR)/world/firstWorld.o \
          $(BUILD_DIR)/rendering/shaders.o \
          $(BUILD_DIR)/rendering/skybox.o \
          $(BUILD_DIR)/gameplay/item.o \
          $(BUILD_DIR)/gameplay/inventory.o \
          $(BUILD_DIR)/gameplay/mining.o \
          $(BUILD_DIR)/gameplay/dropped_item.o

# Target executable
TARGET = $(BUILD_DIR)/game

# Default target
all: $(TARGET)

# Link
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Compile main
$(BUILD_DIR)/main.o: $(SRC_DIR)/main.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/main.cpp -o $(BUILD_DIR)/main.o

# Compile core
$(BUILD_DIR)/core/%.o: $(CORE_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)/core
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile world
$(BUILD_DIR)/world/%.o: $(WORLD_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)/world
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile rendering
$(BUILD_DIR)/rendering/%.o: $(RENDERING_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)/rendering
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile gameplay
$(BUILD_DIR)/gameplay/%.o: $(GAMEPLAY_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)/gameplay
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean
clean:
	rm -rf $(BUILD_DIR)

# Run
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
