# Compiler and flags
CC ?= gcc
CFLAGS ?= -Wall -Wextra -Isrc

# Executable name
EXEC ?= sprache

# Directories
SRC_DIR ?= src
BUILD_DIR ?= build

ifdef DEFINES
	CFLAGS += $(DEFINES)
endif

# List of source files
SRC_FILES := $(wildcard $(SRC_DIR)/*.c)

# Object files derived from source files
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES))


all: $(shell mkdir $(BUILD_DIR)) $(BUILD_DIR)/$(EXEC)

# Rule to build the executable inside the build directory
$(BUILD_DIR)/$(EXEC): $(OBJ_FILES)
	$(CC) $(CFLAGS) $^ -o $@

# Rule to build object files from source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
		$(CC) $(CFLAGS) -c $< -o $@
