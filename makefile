# flags (-g3 for debug options)
CFLAGS ?= -Wall -Wextra -std=c11 -g3

# executable name
TARGET ?= out

# directories
SRC_DIR ?= src
HEADER_DIR ?= include
BUILD_DIR ?= build

CFLAGS += -I$(HEADER_DIR)

ifdef DEFINES
	CFLAGS += $(DEFINES)
endif

ifdef TEST
	CFLAGS += "-DTEST"
endif

SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRC_FILES))

# default target
all: all

# create build folder
$(BUILD_DIR):
	mkdir -p "$@"

# build the executable inside the build directory
$(BUILD_DIR)/$(TARGET): $(OBJ_FILES)
	gcc $(CFLAGS) $^ -o $@

# build object files from source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	gcc $(CFLAGS) -c $< -o $@

.PHONY: all
all: $(BUILD_DIR) $(BUILD_DIR)/$(TARGET)

.PHONY: run
run: all
	./$(BUILD_DIR)/$(TARGET)
