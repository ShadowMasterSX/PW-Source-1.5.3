# Optimized Makefile Template

# Compiler and Flags
CC = clang
CFLAGS = -std=c11 -Wall -Wextra -O2

# Directories
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Files
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_FILES))

# Default Target
.PHONY: all
all: $(BIN_DIR)/app

# Linking
$(BIN_DIR)/app: $(OBJ_FILES)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^
	@echo "[INFO] Linking complete: $@"

# Compilation
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<
	@echo "[INFO] Compiled: $< -> $@"

# Clean
.PHONY: clean
clean:
	@rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "[INFO] Cleaned build artifacts."

# Help
.PHONY: help
help:
	@echo "Makefile Targets:"
	@echo "  all      - Build the application"
	@echo "  clean    - Remove build artifacts"
	@echo "  help     - Display this help message"
