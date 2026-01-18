# Configurazione
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
INCLUDES = -Isrc -Iinclude
LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

# File sorgenti (aggiungi qui i nuovi .cpp)
SOURCES = src/main.cpp \
          src/core/player.cpp \
          src/world/firstWorld.cpp \
          src/rendering/skybox.cpp

# Target
TARGET = build/game

# Compila tutto
all:
	@mkdir -p build
	$(CXX) $(CXXFLAGS) $(SOURCES) $(INCLUDES) -o $(TARGET) $(LIBS)
	@echo "✅ Build completato!"

# Compila ed esegui
run: all
	@./$(TARGET)

# Pulisci
clean:
	@rm -rf build
	@echo "🗑️  Pulito!"

# Rebuild
rebuild: clean all

.PHONY: all run clean rebuild
