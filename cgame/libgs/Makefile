# Optimized Makefile for Sample Project

# Compiler and Flags
CC = gcc
CFLAGS = -Wall -Wextra -O2 -g

# Directories
SRCDIR = src
BINDIR = bin
OBJDIR = obj

# Source and Object Files
SOURCES := $(wildcard $(SRCDIR)/*.c)
OBJECTS := $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SOURCES))

# Default Target
.PHONY: all
all: $(BINDIR)/program

# Linking
$(BINDIR)/program: $(OBJECTS)
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $^
	@echo "[INFO] Linking complete: $@"

# Compilation
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<
	@echo "[INFO] Compiled: $< -> $@"

# Clean Up
.PHONY: clean
clean:
	@rm -rf $(BINDIR) $(OBJDIR)
	@echo "[INFO] Cleaned build artifacts."

# Run Target
.PHONY: run
run: all
	@$(BINDIR)/program

# Help Target
.PHONY: help
help:
	@echo "Makefile Targets:"
	@echo "  all      - Build the program"
	@echo "  run      - Build and execute the program"
	@echo "  clean    - Remove build artifacts"
	@echo "  help     - Display this help message"
