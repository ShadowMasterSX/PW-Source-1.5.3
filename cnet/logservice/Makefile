# Optimized Basic Makefile Example

# Compiler
CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -O2

# Target
TARGET = my_program
SRC = source.c

# Default Target
.PHONY: all
all: $(TARGET)

# Build Target
$(TARGET): $(SRC)
	@$(CC) $(CFLAGS) -o $@ $^
	@echo "[INFO] Build complete: $@"

# Clean
.PHONY: clean
clean:
	@rm -f $(TARGET)
	@echo "[INFO] Cleaned build artifacts."

# Help
.PHONY: help
help:
	@echo "Makefile Targets:"
	@echo "  all      - Build the program"
	@echo "  clean    - Remove build artifacts"
	@echo "  help     - Display this help message"
