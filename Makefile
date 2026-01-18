# Makefile per Dimensional World
# -------------------------------------------------

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra
INCLUDES := -Isrc -Iinclude
LIBS     := -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -lstdc++
BUILD    := build
TARGET   := $(BUILD)/game

# Tutti i file sorgente
SRCS := $(wildcard src/*.cpp) \
        $(wildcard src/core/*.cpp) \
        $(wildcard src/world/*.cpp) \
        $(wildcard src/rendering/*.cpp)

# File oggetto corrispondenti
OBJS := $(SRCS:src/%.cpp=$(BUILD)/%.o)

# -------------------------------------------------
# Regole principali
# -------------------------------------------------

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)

# Compila ogni .cpp in .o
$(BUILD)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Pulizia
clean:
	rm -rf $(BUILD)

# Esegui il gioco
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
