# Optimized Makefile Project

# Compiler and Directories
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
SRCDIR = src
BUILDDIR = build
TARGET = app

# Source and Object Files
SOURCES := $(wildcard $(SRCDIR)/*.cpp)
OBJECTS := $(patsubst $(SRCDIR)/%.cpp, $(BUILDDIR)/%.o, $(SOURCES))

# Default Target
.PHONY: all
all: $(TARGET)

# Build Target
$(TARGET): $(OBJECTS)
	@$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "[INFO] Linking complete: $@"

# Compile Sources
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<
	@echo "[INFO] Compiled: $< -> $@"

# Clean
.PHONY: clean
clean:
	@rm -rf $(BUILDDIR) $(TARGET)
	@echo "[INFO] Cleaned build artifacts."

# Help Target
.PHONY: help
help:
	@echo "Makefile Targets:"
	@echo "  all      - Build the application"
	@echo "  clean    - Remove build artifacts"
	@echo "  help     - Display this help message"
