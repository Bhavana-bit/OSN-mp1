# Compiler and flags
CC = gcc
CFLAGS = -Wall -Iinclude

# Directories
SRC_DIR = src
BIN_DIR = bin

# Source and object files
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(patsubst $(SRC_DIR)/%.c,$(BIN_DIR)/%.o,$(SRC))

# Target executable
TARGET = shell.out

# Default target
all: $(TARGET)

# Create bin directory if it doesn't exist
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Compile .c to .o
$(BIN_DIR)/%.o: $(SRC_DIR)/%.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Link all object files into the final executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

# Run the shell
.PHONY: run
run: $(TARGET)
	./$(TARGET)

# Clean build files
.PHONY: clean
clean:
	rm -rf $(BIN_DIR)/*.o $(TARGET)
