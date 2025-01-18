# Optimized General Server Makefile

# Compiler and Flags
CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2

# Directories
SRC_DIR = server_src
OBJ_DIR = server_obj
BIN_DIR = server_bin

# Files
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SOURCES))

# Default Target
.PHONY: all
all: $(BIN_DIR)/server

# Linking
$(BIN_DIR)/server: $(OBJECTS)
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
	@echo "  all      - Build the server application"
	@echo "  clean    - Remove build artifacts"
	@echo "  help     - Display this help message"