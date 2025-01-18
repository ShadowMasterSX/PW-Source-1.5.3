# Optimized Makefile for Another Project

# Compiler and Flags
CC = g++
CFLAGS = -std=c++17 -Wall -Wextra -O2

# Directories
SRCDIR = source
OBJDIR = build
BINDIR = bin

# Sources and Objects
SRC := $(wildcard $(SRCDIR)/*.cpp)
OBJ := $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRC))

# Default Target
.PHONY: all
all: $(BINDIR)/app

# Linking
$(BINDIR)/app: $(OBJ)
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $^
	@echo "[INFO] Linking complete: $@"

# Compilation
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@
	@echo "[INFO] Compiled: $< -> $@"

# Clean Up
.PHONY: clean
clean:
	@rm -rf $(OBJDIR) $(BINDIR)
	@echo "[INFO] Cleaned build artifacts."

# Debug and Release Targets
.PHONY: debug
.PHONY: release

debug:
	$(MAKE) CFLAGS="$(CFLAGS) -g"
	@echo "[INFO] Build configured for debugging."

release:
	$(MAKE) CFLAGS="$(CFLAGS) -O2"
	@echo "[INFO] Build configured for release."

# Help Target
.PHONY: help
help:
	@echo "Makefile Targets:"
	@echo "  all      - Build the application"
	@echo "  clean    - Remove build artifacts"
	@echo "  debug    - Build with debug information"
	@echo "  release  - Build with optimizations"
	@echo "  help     - Display this help message"
