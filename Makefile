# Makefile for MVM Driver Project

# Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra -fPIC -I./src
LDFLAGS = -lwiringPi

# Directories
SRC_DIR = src
TEST_DIR = test
LIB_DIR = lib
BIN_DIR = bin

# Source files
SRC_FILES = $(SRC_DIR)/r595hc.c $(SRC_DIR)/rpi_modes.c $(SRC_DIR)/MVM_SPI.c
LIB_NAME = libmvmdriver.so
TEST_SRC = $(TEST_DIR)/test.c
TEST_BIN = mvm_test

# Targets
all: lib test

lib: $(SRC_FILES)
	@mkdir -p $(LIB_DIR)
	$(CC) $(CFLAGS) -shared -o $(LIB_DIR)/$(LIB_NAME) $(SRC_FILES) $(LDFLAGS)

test_c: $(TEST_SRC) 
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $(BIN_DIR)/$(TEST_BIN) $(TEST_SRC) $(LDFLAGS) -I$(SRC_DIR)

test_python:
		python3 test/test.py

clean:
	rm -rf $(LIB_DIR) $(BIN_DIR)

.PHONY: all lib test_c test_python clean