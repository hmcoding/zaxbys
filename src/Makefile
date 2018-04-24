CLEAN=rm -rf 

CC=gcc
CFLAGS+=-Wall -g
CPPFLAGS+=-I$(INC_DIR)

INC_DIR=include
SRC_DIR=src
OBJ_DIR=build

SRC_FILES=$(wildcard $(SRC_DIR)/*.c)
OBJ_FILES=$(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_FILES))
TARGET=fat32program

TEST_DIR=test
TEST_FILES=$(wildcard $(TEST_DIR)/*.c)
TEST_TARGETS=$(patsubst %, %.c, $(TEST_FILES))

.PHONY: all build target test clean
target: build
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $(TARGET) $(OBJ_FILES) main.c

all: target test

build: setup $(OBJ_FILES)

setup:
	mkdir -p $(OBJ_DIR)

$(OBJ_FILES):$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

test: $(TEST_TARGETS)

$(TEST_TARGETS): build $(SRC_FILES)
$(TEST_TARGETS):$(TEST_DIR)/%:$(TEST_DIR)/%.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $< $(OBJ_FILES)

clean:
	$(CLEAN) $(OBJ_DIR)
	$(CLEAN) $(TARGET)
	#$(CLEAN) $(TEST_TARGETS)
