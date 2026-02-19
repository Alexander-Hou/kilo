CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99 -Iinclude -g
SRC_DIR = src
BUILD_DIR = build
TARGET = kilo

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

DEPS = include/kilo.h

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(DEPS)
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm -rf $(TARGET) ./build
	@echo "Cleaned up build artifacts."