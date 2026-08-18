CC      ?= gcc
AR      ?= ar
CFLAGS  ?= -Wall -Wextra -std=c11 -g
LDFLAGS ?=

TARGET     ?= out
SRC_DIR    ?= src
HEADER_DIR ?= include
TEST_DIR   ?= tests
BUILD_DIR  ?= build

CPPFLAGS += -I$(HEADER_DIR) -I$(SRC_DIR) -MMD -MP
CPPFLAGS += $(DEFINES)

ifdef ASAN
	CFLAGS  += -fsanitize=address,undefined -fno-omit-frame-pointer
	LDFLAGS += -fsanitize=address,undefined
endif

SRC_FILES := $(shell find $(SRC_DIR) -name '*.c')
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES))

LIB      := $(BUILD_DIR)/libsprache.a
LIB_OBJS := $(filter-out $(BUILD_DIR)/main.o,$(OBJ_FILES))

TEST_SRCS := $(shell find $(TEST_DIR) -name '*_tests.c' 2>/dev/null)
TEST_BINS := $(patsubst $(TEST_DIR)/%.c,$(BUILD_DIR)/tests/%,$(TEST_SRCS))

DEPS := $(OBJ_FILES:.o=.d) $(TEST_BINS:=.d)

.PHONY: all test clean asan run docs

all: $(BUILD_DIR)/$(TARGET)

$(LIB): $(LIB_OBJS)
	$(AR) rcs $@ $^

$(BUILD_DIR)/$(TARGET): $(BUILD_DIR)/main.o $(LIB)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/tests/%: $(TEST_DIR)/%.c $(LIB)
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) -I$(TEST_DIR) $(CFLAGS) $< $(LIB) -o $@ $(LDFLAGS)

test: $(TEST_BINS)
	for t in $(TEST_BINS); do \
		./$$t; \
	done;

asan:
	@$(MAKE) ASAN=1 BUILD_DIR=$(BUILD_DIR)-asan all test

clean:
	rm -rf $(BUILD_DIR) $(BUILD_DIR)-asan

run: all
	@./$(BUILD_DIR)/$(TARGET) $(ARGS)

-include $(DEPS)