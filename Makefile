# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2

# Try to use pkg-config for raylib and curl
RAYLIB_CFLAGS := $(shell pkg-config --cflags raylib 2>/dev/null || echo "-I/usr/local/include")
RAYLIB_LIBS := $(shell pkg-config --libs raylib 2>/dev/null || echo "-lraylib -lm -lpthread -ldl")

CURL_CFLAGS := $(shell pkg-config --cflags libcurl 2>/dev/null)
CURL_LIBS := $(shell pkg-config --libs libcurl 2>/dev/null || echo "-lcurl")

# Link flags for Linux (KIC Environment)
# Raylib dependencies: GL, m, pthread, dl, rt, X11
LDFLAGS_EXTRA = -lGL -lm -lpthread -ldl -lrt -lX11

# Combined flags
CFLAGS += $(RAYLIB_CFLAGS) $(CURL_CFLAGS)
LDFLAGS = $(RAYLIB_LIBS) $(CURL_LIBS) $(LDFLAGS_EXTRA)

# Directories
SRC_DIR = src
BUILD_DIR = build

# Source files
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Output
TARGET = draw_game

# Default target
all: $(BUILD_DIR) $(TARGET)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Link
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Compile
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Run
run: $(TARGET)
	./$(TARGET)

# Clean
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: clean all

# Show configuration (for debugging build issues)
config:
	@echo "CC: $(CC)"
	@echo "CFLAGS: $(CFLAGS)"
	@echo "LDFLAGS: $(LDFLAGS)"
	@echo "SOURCES: $(SOURCES)"

.PHONY: all clean run debug config
