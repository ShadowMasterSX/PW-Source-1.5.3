# Optimized Makefile

# Compiler
CXX = g++
CXXFLAGS = -std=c++14 -Wall -Wextra -O2

# Directories
SRC = source
OBJ = objects
BIN = binary

# Files
SOURCES = $(wildcard $(SRC)/*.cpp)
OBJECTS = $(patsubst $(SRC)/%.cpp, $(OBJ)/%.o, $(SOURCES))

# Default target
.PHONY: all
all: $(BIN)/program

# Linking
$(BIN)/program: $(OBJECTS)
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "[INFO] Linking complete: $@"

# Compilation
$(OBJ)/%.o: $(SRC)/%.cpp
	@mkdir -p $(OBJ)
	$(CXX) $(CXXFLAGS) -c -o $@ $<
	@echo "[INFO] Compiled: $< -> $@"

# Clean
.PHONY: clean
clean:
	@rm -rf $(OBJ) $(BIN)
	@echo "[INFO] Cleaned build artifacts."

# Help
.PHONY: help
help:
	@echo "Makefile Targets:"
	@echo "  all      - Build the program"
	@echo "  clean    - Remove build artifacts"
	@echo "  help     - Display this help message"
