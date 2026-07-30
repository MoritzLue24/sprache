CC = gcc
# flags (-g3 for debug options)
CFLAGS ?= -Wall -Wextra -std=c11 -g

# executable name
TARGET ?= out

# directories
SRC_DIR ?= src
HEADER_DIR ?= include
TEST_DIR ?= tests
BUILD_DIR ?= build

CFLAGS += -I$(HEADER_DIR)

ifdef DEFINES
	CFLAGS += $(DEFINES)
endif

ifdef TEST
	CFLAGS += "-DTEST"
endif

SRC_FILES := $(shell find $(SRC_DIR) -name "*.c")
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRC_FILES))

# all sourcefiles except main.c
LIB_SRC_FILES := $(filter-out $(SRC_DIR)/main.c, $(SRC_FILES))

# all tests/*_tests.c files
TEST_SRC_FILES := $(shell find $(TEST_DIR) -name "*_tests.c" 2>/dev/null)
TEST_BINS := $(patsubst $(TEST_DIR)/%.c, $(BUILD_DIR)/tests/%, $(TEST_SRC_FILES))


all: $(BUILD_DIR)/$(TARGET)

# Link step: link obj files into an executable
$(BUILD_DIR)/$(TARGET): $(OBJ_FILES)
	$(CC) $(CFLAGS) $^ -o $@

# Compile step: compile .c files into .o files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@


.PHONY: test
test: $(TEST_BINS)
	@for t in $(TEST_BINS); do \
		./$$t; \
	done;

# Test-Binary bauen: braucht -I$(TEST_DIR) zusaetzlich zu -I$(HEADER_DIR)
# (fuer tst.h), sonst gleiche CFLAGS wie das Hauptprogramm
$(BUILD_DIR)/tests/%: $(TEST_DIR)/%.c $(LIB_SRC_FILES)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(TEST_DIR) $^ -o $@






# # default target
# all: all


# # create build folder
# $(BUILD_DIR):
# 	mkdir -p "$@"

# # build the executable inside the build directory
# $(BUILD_DIR)/$(TARGET): $(OBJ_FILES)
# 	gcc $(CFLAGS) $^ -o $@

# # build object files from source files
# $(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
# 	gcc $(CFLAGS) -c $< -o $@

# .PHONY: all
# all: $(BUILD_DIR) $(BUILD_DIR)/$(TARGET)

# .PHONY: run
# run: all
# 	./$(BUILD_DIR)/$(TARGET)
