# Optimized Makefile Example

# Compiler and Flags
CC = clang
CFLAGS = -Wall -Wextra -O2

# Directories
SRCDIR = code
OBJDIR = objs
BINDIR = output

# Files
SRCFILES = $(wildcard $(SRCDIR)/*.c)
OBJFILES = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCFILES))

# Default Target
.PHONY: all
all: $(BINDIR)/main

# Build Executable
$(BINDIR)/main: $(OBJFILES)
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $^
	@echo "[INFO] Linking complete: $@"

# Compile Object Files
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<
	@echo "[INFO] Compiled: $< -> $@"

# Clean Up
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
