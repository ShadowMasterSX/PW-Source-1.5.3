# Optimized Makefile

# Compiler and Flags
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2

# Directories
SRC_DIR = sources
BUILD_DIR = build
BIN_DIR = binaries

# Files
SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRC_FILES))

# Default Target
.PHONY: all
all: $(BIN_DIR)/output

# Linker
$(BIN_DIR)/output: $(OBJ_FILES)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "[INFO] Linking complete: $@"

# Compiler
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<
	@echo "[INFO] Compiled: $< -> $@"

# Clean
.PHONY: clean
clean:
	@rm -rf $(BUILD_DIR) $(BIN_DIR)
	@echo "[INFO] Cleaned build artifacts."

# Help
.PHONY: help
help:
	@echo "Makefile Targets:"
	@echo "  all      - Build the application"
	@echo "  clean    - Remove build artifacts"
	@echo "  help     - Display this help message"
