# Optimized Makefile

# Compiler and Flags
CC = gcc
CFLAGS = -Wall -Wextra -O2

# Directories
SRCDIR = src
OBJDIR = obj
BINDIR = bin

# Files
SRC := $(wildcard $(SRCDIR)/*.c)
OBJ := $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRC))

# Default Target
.PHONY: all
all: $(BINDIR)/main

# Build Target
$(BINDIR)/main: $(OBJ)
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $^
	@echo "[INFO] Linking complete: $@"

# Compile Sources
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<
	@echo "[INFO] Compiled: $< -> $@"

# Clean
.PHONY: clean
clean:
	@rm -rf $(OBJDIR) $(BINDIR)
	@echo "[INFO] Cleaned build artifacts."

# Help Target
.PHONY: help
help:
	@echo "Makefile Targets:"
	@echo "  all      - Build the application"
	@echo "  clean    - Remove build artifacts"
	@echo "  help     - Display this help message"
