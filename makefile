
# ----------------------------
# Makefile for minishell
# ----------------------------

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -g -O1 -fsanitize=address -fno-omit-frame-pointer

# Source files
SRCS = koshell.c dynarray.c

# Object files (replace .c with .o)
OBJS = $(SRCS:.c=.o)

# Executable name
TARGET = koshell

BUILD_DIR = ./

# Default target
all: $(BUILD_DIR)/$(TARGET)

# Link object files into executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

# Compile each .c into .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up object files and executable
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets
.PHONY: all clean
