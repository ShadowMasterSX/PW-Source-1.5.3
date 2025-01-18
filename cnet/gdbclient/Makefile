# Optimized Makefile Example

# Compiler and Flags
CC = gcc
CFLAGS = -Wall -Wextra -O2

# Directories
SRC_DIR = source
OBJ_DIR = build
BIN_DIR = output

# Files
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SOURCES))

# Default Target
.PHONY: all
all: $(BIN_DIR)/app

# Linking
$(BIN_DIR)/app: $(OBJECTS)
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
